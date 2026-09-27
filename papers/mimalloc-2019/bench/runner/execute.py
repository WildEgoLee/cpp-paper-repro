"""Run sealed invocations. Equivalence keys are not consulted.

Foreground workloads stop before exec so perf is armed on the known pid.
Redis is a different window: the server is a direct child, and perf covers
only the client request interval.
"""

from __future__ import annotations

import json
import os
import signal
import subprocess
import time
from pathlib import Path
from typing import Any, Callable

from .artifacts import lookup_token, substitute
from .env import child_env
from .identity import seal
from .launch import launch_foreground, spawn_exec
from .maps import read_proc_maps, verify_maps_text
from .perf import PerfSession, coverage_from_session
from .plan import invocation_id, scheduled_invocations
from .resume import pending, write_case

PerfFactory = Callable[[Path], Any]


def execute_run(
    run_dir: Path,
    repo_sha: str,
    *,
    parent_env: dict[str, str] | None = None,
    perf_factory: PerfFactory | None = None,
    limit: int | None = None,
) -> dict[str, Any]:
    seal(run_dir, repo_sha)
    plan = json.loads((run_dir / "plan.json").read_text())
    artifacts = json.loads((run_dir / "artifacts.json").read_text())
    # Touch the scheduler so a deduplicating plan cannot reach the loop.
    scheduled_invocations(plan)
    ran = 0
    for item in pending(plan, run_dir):
        if limit is not None and ran >= limit:
            break
        execute_item(
            run_dir,
            item,
            artifacts,
            parent_env=parent_env if parent_env is not None else dict(os.environ),
            perf_factory=perf_factory,
        )
        ran += 1
    return {"ran": ran, "pending": len(pending(plan, run_dir))}


def execute_item(
    run_dir: Path,
    item: dict[str, Any],
    artifacts: dict[str, Any],
    *,
    parent_env: dict[str, str],
    perf_factory: PerfFactory | None = None,
) -> dict[str, Any]:
    command = item["command"]
    if command["workload"] == "redis":
        record = execute_redis(run_dir, item, artifacts, parent_env, perf_factory)
    else:
        record = execute_foreground(run_dir, item, artifacts, parent_env, perf_factory)
    record["key"] = item["key"]
    record["resume_identity"] = invocation_id(item["key"])
    record["execution_equivalence_key"] = item.get("execution_equivalence_key")
    record["equivalence_key_used_for_scheduling"] = False
    write_case(run_dir, record)
    return record


def execute_foreground(
    run_dir: Path,
    item: dict[str, Any],
    artifacts: dict[str, Any],
    parent_env: dict[str, str],
    perf_factory: PerfFactory | None,
) -> dict[str, Any]:
    command = item["command"]
    argv = substitute(command["argv"], artifacts)
    assert argv is not None
    preload = _planned_preload(command, artifacts)
    env, delta = child_env(parent_env, preload)
    attempt = _next_attempt(run_dir, item["key"])
    present, absent = _map_needles(command, artifacts)
    perf = _make_perf(perf_factory, attempt / "perf.txt")
    maps_box: dict[str, Any] = {"text": "", "check": _failed_check("not checked")}

    def while_stopped(pid: int) -> None:
        perf.attach(pid)

    def after_exec(pid: int, maps_text: str) -> None:
        maps_box["text"] = maps_text
        (attempt / "maps.txt").write_text(maps_text)
        maps_box["check"] = verify_maps_text(maps_text, present, absent)

    try:
        launched = launch_foreground(
            argv,
            env,
            attempt / "stdout",
            attempt / "stderr",
            while_stopped,
            after_exec,
            ready=_maps_ready(present),
        )
    except Exception as exc:
        return _incomplete(attempt, argv, delta, error=str(exc))
    perf_text = perf.stop()
    (attempt / "perf.txt").write_text(perf_text)
    return _record_from_process(
        attempt,
        argv,
        delta,
        launched,
        maps_box["check"],
        perf,
        perf_text,
        trace=launched.get("trace") or [],
    )


def execute_redis(
    run_dir: Path,
    item: dict[str, Any],
    artifacts: dict[str, Any],
    parent_env: dict[str, str],
    perf_factory: PerfFactory | None,
    *,
    client_timeout: float = 30.0,
) -> dict[str, Any]:
    command = item["command"]
    server_argv = substitute(command["server_argv"], artifacts)
    client_argv = substitute(command["client_argv"], artifacts)
    ping_argv = substitute(command["ping_argv"], artifacts)
    shutdown_argv = substitute(command["shutdown_argv"], artifacts)
    assert server_argv and client_argv and ping_argv and shutdown_argv
    preload = _planned_preload(command, artifacts)
    server_env, server_delta = child_env(parent_env, preload)
    client_env, client_delta = child_env(parent_env, None)
    attempt = _next_attempt(run_dir, item["key"])
    present, absent = _map_needles(command, artifacts)
    perf = _make_perf(perf_factory, attempt / "perf.txt")
    trace = ["server_spawn"]
    pid = spawn_exec(server_argv, server_env, attempt / "server.stdout", attempt / "server.stderr")
    client_code = 1
    client_elapsed = None
    maps_check = _failed_check("not checked")
    perf_text = ""
    try:
        if not _wait_pong(ping_argv, client_env):
            trace.append("ping_failed")
            raise RuntimeError("redis ping did not return PONG")
        trace.append("ping_ok")
        maps_text = read_proc_maps(pid)
        (attempt / "maps.txt").write_text(maps_text)
        maps_check = verify_maps_text(maps_text, present, absent)
        trace.append("maps_checked")
        perf.attach(pid)
        trace.append("perf_armed")
        client_started = time.perf_counter()
        client = subprocess.run(
            client_argv,
            env=client_env,
            check=False,
            capture_output=True,
            timeout=client_timeout,
        )
        client_elapsed = time.perf_counter() - client_started
        (attempt / "client.stdout").write_bytes(client.stdout)
        (attempt / "client.stderr").write_bytes(client.stderr)
        client_code = client.returncode
        trace.append("client_done")
        perf_text = perf.stop()
        trace.append("perf_stopped")
        subprocess.run(shutdown_argv, env=client_env, check=False, capture_output=True, timeout=5)
        trace.append("shutdown_sent")
        _wait_until_exited(pid, timeout=2.0)
    except Exception as exc:
        trace.append(f"error:{exc}")
        perf_text = perf.stop()
    finally:
        server_code, usage = _reap_server(pid)
        trace.append("server_reaped")
    (attempt / "perf.txt").write_text(perf_text)
    launched = {
        "exit_code": 0 if client_code == 0 and server_code == 0 else 1,
        "wall_time_s": client_elapsed,
        "rusage": usage,
        "saw_exec": maps_check.get("ok") is True,
    }
    record = _record_from_process(attempt, server_argv, server_delta, launched, maps_check, perf, perf_text, trace)
    record["metrics"]["wall_time"]["window"] = "request-window"
    record["metrics"]["peak_rss"]["window"] = "lifetime"
    record["client_argv"] = client_argv
    record["client_exit_code"] = client_code
    record["server_exit_code"] = server_code
    record["client_env_delta"] = client_delta
    if client_code == 0 and server_code == 0:
        record["exit_code"] = 0
    elif server_code not in (0, None):
        record["exit_code"] = server_code
    else:
        record["exit_code"] = client_code or 1
    return record


def _record_from_process(
    attempt: Path,
    argv: list[str],
    delta: dict[str, Any],
    launched: dict[str, Any],
    maps_check: dict[str, Any],
    perf: Any,
    perf_text: str,
    trace: list[str],
) -> dict[str, Any]:
    parsed = coverage_from_session(getattr(perf, "status", "error"), perf_text)
    usage = launched.get("rusage")
    kib = None if usage is None else usage.ru_maxrss
    wall = launched.get("wall_time_s")
    coverage = {
        "wall_time": "ok" if wall is not None else "parse-error",
        "peak_rss": "ok" if kib is not None else "parse-error",
    }
    coverage.update(parsed["coverage"])
    session = getattr(perf, "status", "error")
    metrics_parsed = bool(parsed["parsed"]) and session != "error"
    return {
        "argv": argv,
        "env_delta": delta,
        "exit_code": launched.get("exit_code", 1),
        "trace": trace,
        "raw_dir": str(attempt),
        "allocator_check": maps_check,
        "metric_coverage": coverage,
        "metrics": {
            "parsed": metrics_parsed,
            "wall_time": {"status": coverage["wall_time"], "seconds": wall, "window": "lifetime", "clock": "release_to_reap"},
            "peak_rss": {
                "status": coverage["peak_rss"],
                "ru_maxrss_kib": kib,
                "bytes": None if kib is None else kib * 1024,
            },
            "perf": {
                "session": session,
                "reason": getattr(perf, "reason", ""),
                "coverage": parsed["coverage"],
                "values": parsed["values"],
            },
        },
    }


def _incomplete(attempt: Path, argv: list[str], delta: dict[str, Any], *, error: str) -> dict[str, Any]:
    return {
        "argv": argv,
        "env_delta": delta,
        "exit_code": 1,
        "trace": ["launch_error"],
        "raw_dir": str(attempt),
        "error": error,
        "allocator_check": _failed_check(error),
        "metric_coverage": {"wall_time": "parse-error", "peak_rss": "parse-error"},
        "metrics": {"parsed": False, "perf": {"session": "error", "reason": error, "coverage": {}, "values": {}}},
    }


def _maps_ready(present: list[str]) -> Callable[[str], bool]:
    def ready(text: str) -> bool:
        if present:
            return all(path in text for path in present)
        return "libc.so.6" in text

    return ready


def _planned_preload(command: dict[str, Any], artifacts: dict[str, Any]) -> str | None:
    raw = command.get("env", {}).get("LD_PRELOAD")
    if not raw:
        return None
    return lookup_token(raw, artifacts)


def _map_needles(command: dict[str, Any], artifacts: dict[str, Any]) -> tuple[list[str], list[str]]:
    check = command["allocator_check"]
    present = [lookup_token(token, artifacts) for token in check.get("require_present", [])]
    absent: list[str] = []
    for token in check.get("require_absent", []):
        if token.startswith("{lib:") and token.endswith("}"):
            name = token[len("{lib:") : -1]
            entry = artifacts["allocators"][name]
            if entry.get("path"):
                absent.append(entry["path"])
            base = entry.get("expected_basename")
            if base:
                absent.append("/" + base)
        else:
            absent.append(lookup_token(token, artifacts))
    return present, absent


def _make_perf(factory: PerfFactory | None, out_path: Path) -> Any:
    if factory is None:
        return PerfSession(out_path)
    return factory(out_path)


def _next_attempt(run_dir: Path, key: dict[str, Any]) -> Path:
    base = run_dir / "raw" / invocation_id(key).replace("/", "__")
    base.mkdir(parents=True, exist_ok=True)
    number = 1
    while (base / f"attempt-{number:02d}").exists():
        number += 1
    path = base / f"attempt-{number:02d}"
    path.mkdir()
    return path


def _wait_pong(argv: list[str], env: dict[str, str], timeout: float = 5.0) -> bool:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        ran = subprocess.run(argv, env=env, check=False, capture_output=True, timeout=2)
        if b"PONG" in ran.stdout or b"PONG" in ran.stderr:
            return True
        time.sleep(0.05)
    return False


def _wait_until_exited(pid: int, timeout: float) -> None:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        stat = Path(f"/proc/{pid}/stat")
        if not stat.exists():
            return
        fields = stat.read_text().split()
        if len(fields) > 2 and fields[2] == "Z":
            return
        time.sleep(0.05)


def _reap_server(pid: int) -> tuple[int, Any]:
    try:
        got, status, usage = os.wait4(pid, os.WNOHANG)
    except ChildProcessError:
        return 1, None
    if got == 0:
        _signal(pid, signal.SIGTERM)
        time.sleep(0.2)
        try:
            got, status, usage = os.wait4(pid, os.WNOHANG)
        except ChildProcessError:
            return 1, None
        if got == 0:
            _signal(pid, signal.SIGKILL)
            try:
                _got, status, usage = os.wait4(pid, 0)
            except ChildProcessError:
                return 1, None
    if os.WIFEXITED(status):
        return os.WEXITSTATUS(status), usage
    return 128 + os.WTERMSIG(status), usage


def _signal(pid: int, sig: int) -> None:
    try:
        os.kill(pid, sig)
    except ProcessLookupError:
        pass


def _failed_check(reason: str) -> dict[str, Any]:
    return {"ok": False, "method": "proc_maps", "matched_lines": [], "problems": [{"error": reason}]}
