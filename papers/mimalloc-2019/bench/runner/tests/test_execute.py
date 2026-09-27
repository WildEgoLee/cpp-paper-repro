import json
import os
import subprocess
import tempfile
import textwrap
import unittest
from pathlib import Path

from runner.artifacts import bind_manifest, file_sha256, probe_glibc
from runner.cli import main
from runner.env import child_env
from runner.execute import execute_item
from runner.identity import IdentityMismatch, seal
from runner.maps import verify_maps_text as maps_verify
from runner.perf import parse_perf_stat, perf_argv
from runner.plan import expand, scheduled_invocations
from runner.resume import is_complete, pending


class PolicyTest(unittest.TestCase):
    def test_equivalence_key_does_not_schedule(self):
        plan = expand("round1")
        policy = plan["execution_equivalence_key_policy"]
        self.assertTrue(policy["analysis_only"])
        self.assertFalse(policy["deduplicate_execution"])
        self.assertEqual(policy["resume_identity"], "invocation_id")
        scheduled = scheduled_invocations(plan)
        self.assertEqual(len(scheduled), 1728)
        self.assertEqual(len({item["execution_equivalence_key"] for item in scheduled}), 76)
        self.assertGreater(len(scheduled), len({item["execution_equivalence_key"] for item in scheduled}))


class ParseTest(unittest.TestCase):
    def test_unsupported_is_not_a_parser_failure(self):
        text = "\n".join(
            [
                "100;;cycles",
                "<not supported>;;cache-misses",
                "<not counted>;;instructions",
            ]
        )
        parsed = parse_perf_stat(text)
        self.assertTrue(parsed["parsed"])
        self.assertEqual(parsed["coverage"]["cycles"], "ok")
        self.assertEqual(parsed["coverage"]["cache-misses"], "unsupported")
        self.assertEqual(parsed["coverage"]["instructions"], "unavailable")

    def test_garbage_is_a_parser_failure(self):
        self.assertFalse(parse_perf_stat("not a perf file\n").get("parsed"))

    def test_perf_follows_new_threads(self):
        argv = perf_argv(10, Path("perf.txt"))
        self.assertNotIn("--no-inherit", argv)


class MapsAndEnvTest(unittest.TestCase):
    def test_basename_is_not_a_match(self):
        wrong = "7f000-7f100 r-xp 00000000 00:00 1 /tmp/wrong/libjemalloc.so\n"
        self.assertFalse(maps_verify(wrong, ["/tmp/right/libjemalloc.so"], [])["ok"])
        right = "7f000-7f100 r-xp 00000000 00:00 1 /tmp/right/libjemalloc.so\n"
        checked = maps_verify(right, ["/tmp/right/libjemalloc.so"], ["/tmp/wrong/libjemalloc.so"])
        self.assertTrue(checked["ok"])
        self.assertIn("/tmp/right/libjemalloc.so", checked["matched_lines"][0])

    def test_parent_preload_is_removed(self):
        env, delta = child_env({"PATH": "/usr/bin", "LD_PRELOAD": "/tmp/evil.so"}, "/opt/libjemalloc.so")
        self.assertNotIn("/tmp/evil.so", env.values())
        self.assertEqual(env["LD_PRELOAD"], "/opt/libjemalloc.so")
        self.assertEqual(delta["removed"]["LD_PRELOAD"], "/tmp/evil.so")
        glibc_env, glibc_delta = child_env(env, None)
        self.assertNotIn("LD_PRELOAD", glibc_env)
        self.assertEqual(glibc_delta["applied"], {})


class LiveExecutionTest(unittest.TestCase):
    def test_body_starts_only_after_measurement_and_maps_match_the_file(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            sleeper, library = _build_targets(root)
            (root / "libmimalloc.so").write_bytes(b"mi")
            (root / "libtcmalloc.so").write_bytes(b"tc")
            order = root / "order"
            fakes = []

            def factory(out_path):
                perf = _FakePerf(order)
                fakes.append(perf)
                return perf

            artifacts = bind_manifest(
                {"alloc-test": sleeper},
                {
                    "jemalloc": library,
                    "mimalloc-v1.0.0": root / "libmimalloc.so",
                    "tcmalloc": root / "libtcmalloc.so",
                },
                _fake_glibc(root),
            )
            item = _foreground_item(sleeper, order)
            run = root / "run"
            run.mkdir()
            record = execute_item(
                run,
                item,
                artifacts,
                parent_env={"PATH": os.environ.get("PATH", ""), "LD_PRELOAD": "/tmp/not-the-plan.so"},
                perf_factory=factory,
            )
            self.assertTrue(fakes[0].saw_stopped)
            self.assertEqual(order.read_text().splitlines(), ["perf_ready", "body"])
            self.assertEqual(record["trace"][:4], ["child_forked", "child_stopped", "measurement_ready", "child_released"])
            self.assertTrue(record["allocator_check"]["ok"])
            self.assertTrue(any(str(library.resolve()) in line for line in record["allocator_check"]["matched_lines"]))
            self.assertEqual(record["env_delta"]["removed"]["LD_PRELOAD"], "/tmp/not-the-plan.so")
            self.assertEqual(record["env_delta"]["applied"]["LD_PRELOAD"], str(library.resolve()))
            self.assertGreater(record["metrics"]["peak_rss"]["ru_maxrss_kib"], 0)
            self.assertEqual(record["metric_coverage"]["cycles"], "unavailable")
            self.assertTrue(record["metrics"]["parsed"])
            self.assertTrue(is_complete(_reread(run)))
            self.assertTrue((Path(record["raw_dir"]) / "stdout").exists())

    def test_failed_parse_keeps_raw_and_resume_uses_the_invocation(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            sleeper, library = _build_targets(root)
            order = root / "order"

            def factory(out_path):
                return _FakePerf(order, status="error")

            artifacts = bind_manifest({"alloc-test": sleeper}, {"jemalloc": library}, _fake_glibc(root))
            item = _foreground_item(sleeper, order)
            # glibc-style absent list is empty of unresolved tokens when only jemalloc is bound.
            item["command"]["allocator_check"] = {
                "require_present": ["{lib:jemalloc}"],
                "require_absent": [],
            }
            run = root / "run"
            run.mkdir()
            record = execute_item(run, item, artifacts, parent_env={"PATH": "/usr/bin"}, perf_factory=factory)
            self.assertFalse(record["metrics"]["parsed"])
            self.assertTrue((Path(record["raw_dir"]) / "maps.txt").exists())
            plan = {
                "execution_equivalence_key_policy": {
                    "analysis_only": True,
                    "deduplicate_execution": False,
                    "resume_identity": "invocation_id",
                },
                "invocation_list": [item, _same_execution_other_repetition(item)],
            }
            left = pending(plan, run)
            self.assertEqual([entry["key"]["repetition"] for entry in left], [1, 2])
            again = execute_item(run, item, artifacts, parent_env={"PATH": "/usr/bin"}, perf_factory=factory)
            self.assertTrue((run / "raw").exists())
            attempts = list(Path(again["raw_dir"]).parent.glob("attempt-*"))
            self.assertEqual(len(attempts), 2)


class RedisLifecycleTest(unittest.TestCase):
    def test_server_window_and_a_server_failure_are_not_complete(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            scripts = _redis_scripts(root)
            artifacts = bind_manifest({}, {}, _fake_glibc(root))
            ok = execute_item(
                root / "ok",
                _redis_item(scripts, "stay"),
                artifacts,
                parent_env={"PATH": os.environ["PATH"], "LD_PRELOAD": "/tmp/evil.so"},
                perf_factory=lambda out: _FakePerf(root / "unused"),
            )
            self.assertEqual(
                [step for step in ok["trace"] if not step.startswith("error")],
                [
                    "server_spawn",
                    "ping_ok",
                    "maps_checked",
                    "perf_armed",
                    "client_done",
                    "perf_stopped",
                    "shutdown_sent",
                    "server_reaped",
                ],
            )
            self.assertEqual(ok["client_exit_code"], 0)
            self.assertEqual(ok["server_exit_code"], 0)
            self.assertNotIn("LD_PRELOAD", ok["client_env_delta"]["applied"])
            self.assertTrue(is_complete(_stored(root / "ok")))
            bad = execute_item(
                root / "bad",
                _redis_item(scripts, "die"),
                artifacts,
                parent_env={"PATH": os.environ["PATH"]},
                perf_factory=lambda out: _FakePerf(root / "unused2"),
            )
            self.assertNotEqual(bad["server_exit_code"], 0)
            self.assertFalse(is_complete(_stored(root / "bad")))


class IdentityTest(unittest.TestCase):
    def test_resume_refuses_a_changed_artifact_and_dry_run_will_not_overwrite(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            run = root / "run"
            blob = root / "libjemalloc.so"
            blob.write_bytes(b"v1")
            glibc = _fake_glibc(root)
            artifacts = bind_manifest({}, {"jemalloc": blob}, glibc)
            plan = expand("smoke")
            plan["marker"] = "keep"
            run.mkdir()
            (run / "plan.json").write_text(json.dumps(plan))
            (run / "host.json").write_text(json.dumps({"uname": "test", "hardware_threads": 12, "physical_cores": 6, "numa_nodes": 1}))
            (run / "artifacts.json").write_text(json.dumps(artifacts))
            seal(run, "abc")
            blob.write_bytes(b"v2")
            with self.assertRaises(IdentityMismatch):
                seal(run, "abc")
            code = main(["dry-run", "--profile", "smoke", "--out", str(run)])
            self.assertEqual(code, 3)
            self.assertEqual(json.loads((run / "plan.json").read_text())["marker"], "keep")


class GlibcProbeTest(unittest.TestCase):
    def test_system_libc_is_a_real_file_not_a_preload(self):
        probed = probe_glibc()
        self.assertTrue(Path(probed["path"]).exists())
        self.assertEqual(probed["sha256"], file_sha256(Path(probed["path"])))
        self.assertRegex(probed["version"], r"\d+\.\d+")
        self.assertIsNone(bind_manifest({}, {}, probed)["allocators"]["glibc"]["ld_preload"])


class _FakePerf:
    def __init__(self, order: Path, status: str = "unavailable"):
        self.order = order
        self.status = status
        self.reason = "test"
        self.saw_stopped = False

    def attach(self, pid: int) -> None:
        text = Path(f"/proc/{pid}/status").read_text()
        self.saw_stopped = "T (stopped)" in text or "t (tracing stop)" in text
        with self.order.open("a") as handle:
            handle.write("perf_ready\n")

    def stop(self) -> str:
        return ""


def _build_targets(root: Path) -> tuple[Path, Path]:
    sleeper_c = root / "sleeper.c"
    sleeper_c.write_text(
        textwrap.dedent(
            """
            #include <stdio.h>
            #include <stdlib.h>
            #include <unistd.h>
            int main(int argc, char **argv) {
                FILE *f = fopen(argv[1], "a");
                if (!f) return 2;
                fputs("body\\n", f);
                fclose(f);
                usleep((useconds_t)atoi(argv[2]) * 1000);
                return 0;
            }
            """
        )
    )
    lib_c = root / "lib.c"
    lib_c.write_text("void anchor(void) {}\n")
    library = root / "libjemalloc.so"
    sleeper = root / "sleeper"
    subprocess.check_call(["gcc", "-shared", "-fPIC", "-o", str(library), str(lib_c)])
    subprocess.check_call(["gcc", "-o", str(sleeper), str(sleeper_c)])
    return sleeper, library


def _fake_glibc(root: Path) -> dict:
    libc = root / "libc.so.6"
    loader = root / "ld.so"
    libc.write_bytes(b"libc")
    loader.write_bytes(b"ld")
    return {
        "path": str(libc.resolve()),
        "sha256": file_sha256(libc),
        "version": "2.0-test",
        "loader": str(loader.resolve()),
        "loader_sha256": file_sha256(loader),
    }


def _foreground_item(sleeper: Path, order: Path) -> dict:
    return {
        "key": {
            "workload": "alloc-test",
            "allocator": "jemalloc",
            "logical_threads": 1,
            "phase": "measured",
            "repetition": 1,
        },
        "execution_equivalence_key": "alloc-test/jemalloc/workers=1",
        "command": {
            "workload": "alloc-test",
            "allocator": "jemalloc",
            "logical_threads": 1,
            "argv": [str(sleeper), str(order), "400"],
            "env": {"LD_PRELOAD": "{lib:jemalloc}"},
            "allocator_check": {
                "require_present": ["{lib:jemalloc}"],
                "require_absent": ["{lib:mimalloc-v1.0.0}", "{lib:tcmalloc}"],
            },
        },
    }


def _same_execution_other_repetition(item: dict) -> dict:
    other = json.loads(json.dumps(item))
    other["key"]["repetition"] = 2
    return other


def _redis_scripts(root: Path) -> dict[str, Path]:
    server = root / "server.py"
    server.write_text(
        textwrap.dedent(
            """
            import sys, time
            from pathlib import Path
            ready, shutdown, mode = sys.argv[1:]
            Path(ready).write_text("up")
            if mode == "die":
                time.sleep(0.2)
                raise SystemExit(3)
            while not Path(shutdown).exists():
                time.sleep(0.02)
            """
        )
    )
    ping = root / "ping.py"
    ping.write_text(
        textwrap.dedent(
            """
            import sys
            from pathlib import Path
            print("PONG" if Path(sys.argv[1]).exists() else "NO")
            """
        )
    )
    client = root / "client.py"
    client.write_text("print('1000.0 requests per second')\n")
    shutdown = root / "shutdown.py"
    shutdown.write_text(
        textwrap.dedent(
            """
            import sys
            from pathlib import Path
            Path(sys.argv[1]).write_text("down")
            """
        )
    )
    return {"server": server, "ping": ping, "client": client, "shutdown": shutdown}


def _redis_item(scripts: dict[str, Path], mode: str) -> dict:
    py = Path(os.sys.executable) if False else __import__("sys").executable
    ready = scripts["server"].with_name(f"ready-{mode}")
    stop = scripts["server"].with_name(f"stop-{mode}")
    return {
        "key": {
            "workload": "redis",
            "allocator": "glibc",
            "logical_threads": 1,
            "phase": "measured",
            "repetition": 1,
        },
        "execution_equivalence_key": "redis/glibc/not-a-scaling-row",
        "command": {
            "workload": "redis",
            "allocator": "glibc",
            "logical_threads": 1,
            "server_argv": [py, str(scripts["server"]), str(ready), str(stop), mode],
            "client_argv": [py, str(scripts["client"])],
            "ping_argv": [py, str(scripts["ping"]), str(ready)],
            "shutdown_argv": [py, str(scripts["shutdown"]), str(stop)],
            "env": {"LD_PRELOAD": None},
            "allocator_check": {"require_present": [], "require_absent": []},
        },
    }


def _reread(run: Path) -> dict:
    return json.loads(next((run / "cases").rglob("*.json")).read_text())


def _stored(run: Path) -> dict:
    return json.loads(next((run / "cases").rglob("*.json")).read_text())


if __name__ == "__main__":
    unittest.main()
