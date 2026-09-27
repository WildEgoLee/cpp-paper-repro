import json
import tempfile
import unittest
from pathlib import Path

from runner.aliases import UnknownId, canonical_allocator, canonical_workload
from runner.adapters import resolve
from runner.cli import main
from runner.plan import case_path, expand, load_matrix
from runner.preflight import read_host
from runner.resume import is_complete, pending, write_case
from runner.stats import paper_view, protocol_statistics
from runner.validate import validate_resolved


class AliasTest(unittest.TestCase):
    def test_environment_ids_land_on_matrix_ids(self):
        self.assertEqual(canonical_workload("xmalloc-test"), "xmalloc")
        self.assertEqual(canonical_allocator("mi"), "mimalloc-v1.0.0")
        self.assertEqual(canonical_allocator("je"), "jemalloc")
        self.assertEqual(canonical_allocator("tc"), "tcmalloc")

    def test_unknown_ids_are_not_guessed(self):
        with self.assertRaises(UnknownId):
            canonical_workload("xmalloc_test")
        with self.assertRaises(UnknownId):
            canonical_allocator("jemalloc5")


class ResolveTest(unittest.TestCase):
    def test_alloc_test_thread_is_argv(self):
        command = resolve("alloc-test", 8, "glibc")
        self.assertEqual(command["argv"], ["{artifact:alloc-test}", "8"])
        self.assertIsNone(command["env"]["LD_PRELOAD"])
        validate_resolved(command)

    def test_larson_changes_only_the_thread_slot(self):
        command = resolve("larson", 8, "je")
        self.assertEqual(command["argv"][-1], "8")
        self.assertEqual(command["argv"][1:7], ["2.5", "7", "8", "1000", "10000", "42"])
        self.assertEqual(command["parameter_roles"]["paper_point_not_in_matrix"][-1], "100")
        self.assertEqual(command["env"]["LD_PRELOAD"], "{lib:jemalloc}")
        validate_resolved(command)

    def test_xmalloc_does_not_apply_the_extra_doubling(self):
        command = resolve("xmalloc-test", 8, "tc")
        self.assertEqual(command["workload"], "xmalloc")
        argv = command["argv"]
        self.assertEqual(argv[argv.index("-w") + 1], "8")
        self.assertEqual(command["parameter_roles"]["os_threads"], 16)
        self.assertIn("2*procs", command["parameter_roles"]["bench_sh_formula_not_used"])
        validate_resolved(command)

    def test_redis_keeps_logical_threads_out_of_argv(self):
        rows = [resolve("redis", n, "mi") for n in (1, 2, 4, 6, 8, 12)]
        clients = [row["client_argv"] for row in rows]
        self.assertTrue(all(client == clients[0] for client in clients))
        for row in rows:
            self.assertEqual(row["measurement_subject"], "server")
            self.assertEqual(row["thread_effect"], "none")
            self.assertTrue(row["parameter_roles"]["pipeline_depth_is_not_logical_threads"])
            self.assertNotIn("perf", row["client_argv"])
            self.assertEqual(row["env"]["LD_PRELOAD"], "{lib:mimalloc-v1.0.0}")
            validate_resolved(row)


class PlanTest(unittest.TestCase):
    def test_round1_is_1728_and_stable(self):
        first = expand("round1")
        second = expand("round1")
        self.assertEqual(first["configurations"], 96)
        self.assertEqual(first["invocations"], 1728)
        self.assertEqual([item["id"] for item in first["invocation_list"]], [item["id"] for item in second["invocation_list"]])
        self.assertEqual(first["acceptance_statistics"], "protocol_statistics")
        self.assertIn("Must not replace", first["paper_statistics_not_acceptance"]["note"])

    def test_shuffle_is_inside_the_block_only(self):
        plan = expand("round1")
        ids = [item["id"] for item in plan["invocation_list"]]
        # First block is alloc-test / 1 thread: 4 allocators * 18 reps = 72.
        block = ids[:72]
        self.assertTrue(all(item.startswith("alloc-test/") and "/1/" in item for item in block))
        allocators = {item.split("/")[1] for item in block}
        self.assertEqual(allocators, {"glibc", "mimalloc-v1.0.0", "jemalloc", "tcmalloc"})
        # Not grouped as all-glibc then all-mimalloc.
        order = [item.split("/")[1] for item in block]
        self.assertGreater(len(set(order[:18])), 1)

    def test_smoke_is_not_the_matrix(self):
        plan = expand("smoke")
        self.assertEqual(plan["configurations"], 16)
        self.assertEqual(plan["invocations"], 32)
        threads = {item["key"]["logical_threads"] for item in plan["invocation_list"]}
        self.assertEqual(threads, {1})

    def test_matrix_ids_need_no_alias(self):
        matrix = load_matrix()
        for name in matrix["allocators"]:
            self.assertEqual(canonical_allocator(name), name)
        for item in matrix["workloads"]:
            self.assertEqual(canonical_workload(item["id"]), item["id"])


class ResumeTest(unittest.TestCase):
    def test_interrupt_skips_completed_invocations(self):
        plan = expand("smoke")
        with tempfile.TemporaryDirectory() as tmp:
            run = Path(tmp)
            done = plan["invocation_list"][:3]
            for item in done:
                write_case(
                    run,
                    {
                        "key": item["key"],
                        "exit_code": 0,
                        "metrics": {"parsed": True},
                        "allocator_check": {"ok": True},
                    },
                )
            # A crash mid-write is not complete, and a green flag with a bad parser is not either.
            bad = plan["invocation_list"][3]
            write_case(
                run,
                {
                    "key": bad["key"],
                    "exit_code": 0,
                    "metrics": {"parsed": False},
                    "allocator_check": {"ok": True},
                    "complete": True,
                },
            )
            left = pending(plan, run)
            left_ids = [item["id"] for item in left]
            for item in done:
                self.assertNotIn(item["id"], left_ids)
            self.assertIn(bad["id"], left_ids)
            self.assertEqual(len(left), len(plan["invocation_list"]) - 3)
            record = json.loads(case_path(run, done[0]["key"]).read_text())
            self.assertTrue(is_complete(record))


class StatsTest(unittest.TestCase):
    def test_protocol_and_paper_views_stay_apart(self):
        samples = [float(i) for i in range(1, 16)]
        protocol = protocol_statistics(samples)
        self.assertEqual(protocol["name"], "protocol_statistics")
        self.assertEqual(protocol["median"], 8.0)
        self.assertEqual(protocol["p95"], 15.0)
        paper = paper_view([1, 2, 3, 4, 5])
        self.assertEqual(paper["name"], "paper_view")
        self.assertFalse(paper["acceptance"])
        self.assertNotEqual(protocol["name"], paper["name"])


class PreflightTest(unittest.TestCase):
    def test_host_report_does_not_invent_missing_fields(self):
        host = read_host()
        self.assertIn("eligible_for_round_1", host)
        self.assertIn("hardware_threads", host)
        if host["hardware_threads"] is not None and host["hardware_threads"] < 12:
            self.assertFalse(host["eligible_for_round_1"])


class DryRunTest(unittest.TestCase):
    def test_dry_run_writes_plan_without_case_metrics(self):
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / "run"
            code = main(["dry-run", "--profile", "round1", "--out", str(out)])
            self.assertEqual(code, 0)
            plan = json.loads((out / "plan.json").read_text())
            host = json.loads((out / "host.json").read_text())
            artifacts = json.loads((out / "artifacts.json").read_text())
            self.assertEqual(plan["invocations"], 1728)
            self.assertFalse(plan["executes"])
            self.assertIn("eligible_for_round_1", host)
            self.assertEqual(artifacts["benchmarks"]["redis-server"]["status"], "unresolved")
            self.assertEqual(list(out.rglob("cases/**/*.json")), [])


if __name__ == "__main__":
    unittest.main()
