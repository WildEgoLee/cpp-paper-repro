"""Reject resolved commands that would make a thread column lie."""

from __future__ import annotations

from typing import Any


def validate_resolved(command: dict[str, Any]) -> None:
    workload = command["workload"]
    threads = command["logical_threads"]
    roles = command["parameter_roles"]
    if "metrics" not in command or "concurrency" not in command:
        raise ValueError("metrics and concurrency are required")
    _validate_metrics(command)
    _validate_concurrency(command)
    if workload == "redis":
        _validate_redis(command, threads, roles)
    elif workload == "alloc-test":
        if command["argv"] != ["{artifact:alloc-test}", str(threads)]:
            raise ValueError("alloc-test argv is not logical_threads")
        if command["measurement_subject"] != "foreground":
            raise ValueError("alloc-test is a foreground process")
    elif workload == "larson":
        if command["argv"][-1] != str(threads):
            raise ValueError("larson thread count is not argv[7]")
        if command["argv"][1:7] != ["2.5", "7", "8", "1000", "10000", "42"]:
            raise ValueError("larson fixed parameters drifted")
    elif workload == "xmalloc":
        argv = command["argv"]
        if argv[argv.index("-w") + 1] != str(threads):
            raise ValueError("xmalloc -w is not logical_threads")
        if roles["os_threads"] != 2 * threads:
            raise ValueError("xmalloc os_threads must be 2*logical_threads")
        if command["concurrency"]["effective_worker_threads"] != 2 * threads:
            raise ValueError("xmalloc effective_worker_threads must be 2*logical_threads")
        if command["concurrency"]["scaling_report_uses"] != "effective_worker_threads":
            raise ValueError("xmalloc scaling reports must not use the matrix column")
        if roles.get("bench_sh_formula_not_used") is None:
            raise ValueError("xmalloc must record the rejected 2*procs formula")
    else:
        raise ValueError(f"unexpected workload {workload}")
    if command["allocator"] == "glibc" and command["env"].get("LD_PRELOAD") is not None:
        raise ValueError("glibc must not set LD_PRELOAD")
    if command["allocator"] != "glibc" and command["env"].get("LD_PRELOAD") != "{lib:%s}" % command["allocator"]:
        raise ValueError("LD_PRELOAD does not name the canonical allocator")


def _validate_redis(command: dict[str, Any], threads: int, roles: dict[str, Any]) -> None:
    if command["measurement_subject"] != "server":
        raise ValueError("redis measurement_subject must be the server, not the client")
    if command["thread_effect"] != "none":
        raise ValueError("redis logical_threads must not change argv")
    if roles.get("logical_threads_controls_argv") is not False:
        raise ValueError("redis must declare that logical_threads does not control argv")
    if roles.get("pipeline_depth_is_not_logical_threads") is not True:
        raise ValueError("redis -P must be labeled as pipeline depth")
    client = command["client_argv"]
    if "-P" not in client or client[client.index("-P") + 1] != "8":
        raise ValueError("redis pipeline depth drifted from bench.sh")
    # The dangerous case: logical_threads happens to be 8, so a later reader
    # can mistake -P 8 for the matrix column. The role flag is what prevents that.
    if threads == 8 and roles["pipeline_depth"] != 8:
        raise ValueError("pipeline depth and logical_threads collided without a label")
    joined = " ".join(client)
    if "perf" in joined:
        raise ValueError("perf must not wrap redis-benchmark")
    if command["server_argv"] != ["{artifact:redis-server}"]:
        raise ValueError("redis server argv drifted")
    if command["concurrency"]["scaling_row"] is not False:
        raise ValueError("redis is not a scaling row")
    if command["concurrency"]["effective_worker_threads"] is not None:
        raise ValueError("redis has no worker-thread count")
    if command["metrics"]["perf"] != {"subject": "server", "window": "request-window"}:
        raise ValueError("redis perf must be the server request window")
    if command["metrics"]["throughput"]["subject"] != "client":
        raise ValueError("redis throughput belongs to the client")
    if command["metrics"]["peak_rss"] != {"subject": "server", "window": "lifetime"}:
        raise ValueError("redis peak RSS is the server lifetime")


def _validate_metrics(command: dict[str, Any]) -> None:
    for name, spec in command["metrics"].items():
        if spec["subject"] not in ("foreground", "server", "client"):
            raise ValueError(f"{name} has no subject")
        if spec["window"] not in ("lifetime", "request-window"):
            raise ValueError(f"{name} has no window")
    if command["workload"] != "redis":
        if command["metrics"]["perf"]["window"] != "lifetime":
            raise ValueError("foreground perf must cover the process lifetime")
        if "perf_stat_on_foreground_pid" in command["lifecycle"]:
            raise ValueError("perf must not attach after the process has already started")


def _validate_concurrency(command: dict[str, Any]) -> None:
    concurrency = command["concurrency"]
    flag = concurrency["oversubscribed"]
    if flag is not None and not isinstance(flag, bool):
        raise ValueError("oversubscribed must be true, false, or null")
    if not concurrency["scaling_row"] and flag is not None:
        raise ValueError("a non-scaling row cannot be marked oversubscribed")
