"""Map (workload, logical_threads) to argv, env, and who gets measured.

logical_threads is the matrix column. It is not a flag shared by all four
workloads. Redis ignores it. Larson puts it in argv slot 7. xmalloc uses it
as -w, which starts that many producers and the same number of consumers.

Invocation text for the non-thread arguments is taken from mimalloc-bench
bench.sh at e2a9035fb7b0 (2019-06-23), the first script that names these
commands. That commit is not the suite lock (the lock stays 874d1b83). It is
cited because the locked tree has no bench.sh yet, and it predates the
rejected gperftools-2.7 switch.
"""

from __future__ import annotations

from typing import Any

from .aliases import canonical_allocator, canonical_workload

# Larson parameters other than the thread count. Slot 7 in the historical
# script is 100, which is the paper point, not the 1..12 sweep.
LARSON_PREFIX = ["2.5", "7", "8", "1000", "10000", "42"]

# redis-benchmark -P is pipeline depth. It is not logical_threads.
REDIS_PIPELINE_DEPTH = 8
REDIS_REQUESTS = 1_000_000


def resolve(workload: str, logical_threads: int, allocator: str) -> dict[str, Any]:
    """Return the exact command and lifecycle for one configuration."""
    workload_id = canonical_workload(workload)
    allocator_id = canonical_allocator(allocator)
    if not isinstance(logical_threads, int) or isinstance(logical_threads, bool) or logical_threads < 1:
        raise ValueError(f"logical_threads must be a positive int, got {logical_threads!r}")

    env = _preload(allocator_id)
    if workload_id == "alloc-test":
        body = _alloc_test(logical_threads)
    elif workload_id == "larson":
        body = _larson(logical_threads)
    elif workload_id == "xmalloc":
        body = _xmalloc(logical_threads)
    elif workload_id == "redis":
        body = _redis(logical_threads)
    else:
        raise AssertionError(workload_id)

    resolved = {
        "workload": workload_id,
        "allocator": allocator_id,
        "logical_threads": logical_threads,
        "env": env,
        "allocator_check": _allocator_check(allocator_id, body_subject(workload_id)),
        **body,
    }
    return resolved


def body_subject(workload_id: str) -> str:
    return "server" if workload_id == "redis" else "foreground"


def _concurrency(logical_threads: int, workers: int | None, scaling_row: bool, axis: str | None) -> dict[str, Any]:
    return {
        "logical_threads": logical_threads,
        "effective_worker_threads": workers,
        "scaling_row": scaling_row,
        "scaling_report_uses": axis,
        "hardware_threads": None,
        "oversubscribed": None,
    }


def stamp_oversubscription(command: dict[str, Any], hardware_threads: int | None) -> None:
    """Fill oversubscribed from the host. None means the row is not a scaling row, or the host count is unknown."""
    concurrency = command["concurrency"]
    concurrency["hardware_threads"] = hardware_threads
    workers = concurrency["effective_worker_threads"]
    if not concurrency["scaling_row"] or workers is None or hardware_threads is None:
        concurrency["oversubscribed"] = None
        return
    concurrency["oversubscribed"] = workers > hardware_threads


def _preload(allocator_id: str) -> dict[str, Any]:
    if allocator_id == "glibc":
        return {"LD_PRELOAD": None}
    return {"LD_PRELOAD": "{lib:%s}" % allocator_id}


def _allocator_check(allocator_id: str, subject: str) -> dict[str, Any]:
    others = ["mimalloc-v1.0.0", "jemalloc", "tcmalloc"]
    if allocator_id == "glibc":
        return {
            "method": "proc_maps",
            "subject": subject,
            "require_absent": ["{lib:%s}" % name for name in others],
            "require_present": [],
        }
    return {
        "method": "proc_maps",
        "subject": subject,
        "require_present": ["{lib:%s}" % allocator_id],
        "require_absent": ["{lib:%s}" % name for name in others if name != allocator_id],
    }


FOREGROUND_METRICS = {
    "wall_time": {"subject": "foreground", "window": "lifetime"},
    "peak_rss": {"subject": "foreground", "window": "lifetime"},
    "perf": {"subject": "foreground", "window": "lifetime"},
}

# perf wraps the process from exec. Attaching after spawn misses startup and
# races short workloads, so that order is not the contract.
FOREGROUND_LIFECYCLE = [
    "exec_under_perf_stat_covering_lifetime",
    "verify_allocator_while_process_is_alive",
    "wait_exit",
    "parse_lifetime_wall_time_peak_rss_and_perf",
]


def _foreground(
    argv: list[str],
    thread_effect: str,
    roles: dict[str, Any],
    notes: list[str],
    logical_threads: int,
    workers: int,
) -> dict[str, Any]:
    return {
        "measurement_subject": "foreground",
        "thread_effect": thread_effect,
        "argv": argv,
        "server_argv": None,
        "client_argv": None,
        "parameter_roles": roles,
        "metrics": FOREGROUND_METRICS,
        "concurrency": _concurrency(logical_threads, workers, True, "logical_threads" if workers == logical_threads else "effective_worker_threads"),
        "lifecycle": list(FOREGROUND_LIFECYCLE),
        "notes": notes,
    }


def _alloc_test(logical_threads: int) -> dict[str, Any]:
    # bench/CMakeLists.txt defines BENCH, so argc==2 selects threadCount.
    # iterCount is compiled in as 100000000. maxItems is (1<<20)/threadCount.
    return _foreground(
        ["{artifact:alloc-test}", str(logical_threads)],
        thread_effect="argv",
        roles={
            "argv_1": "thread_count",
            "iter_count_compiled": 100_000_000,
            "max_items": "1048576 / logical_threads",
        },
        notes=[
            "logical_threads is argv[1] and the worker count. The binary then divides maxItems by that count.",
        ],
        logical_threads=logical_threads,
        workers=logical_threads,
    )


def _larson(logical_threads: int) -> dict[str, Any]:
    argv = ["{artifact:larson}", *LARSON_PREFIX, str(logical_threads)]
    return _foreground(
        argv,
        thread_effect="argv",
        roles={
            "argv_1": "sleep_sec",
            "argv_2": "min_size",
            "argv_3": "max_size",
            "argv_4": "chunks_per_thread",
            "argv_5": "rounds",
            "argv_6": "seed",
            "argv_7": "max_threads_and_min_threads",
            "paper_point_not_in_matrix": ["{artifact:larson}", *LARSON_PREFIX, "100"],
        },
        notes=[
            "logical_threads replaces only argv[7]. The historical script always passed 100 there and ignored --procs.",
            "min_threads is set equal to max_threads by larson.cpp when argc > 7.",
        ],
        logical_threads=logical_threads,
        workers=logical_threads,
    )


def _xmalloc(logical_threads: int) -> dict[str, Any]:
    # -w is the producer count. The binary starts the same number of consumers.
    # bench.sh passed -w (2*procs), which would make logical_threads=8 mean
    # 16 producers and 16 consumers. That formula is not used for this sweep.
    return _foreground(
        [
            "{artifact:xmalloc-test}",
            "-w",
            str(logical_threads),
            "-t",
            "5",
            "-s",
            "-1",
        ],
        thread_effect="argv",
        roles={
            "dash_w": "producer_threads",
            "consumer_threads": logical_threads,
            "os_threads": 2 * logical_threads,
            "dash_t": "run_seconds",
            "dash_s": "object_size_or_mixed",
            "bench_sh_formula_not_used": "tds=2*procs; -w $tds",
            "paper_point_not_in_matrix": ["{artifact:xmalloc-test}", "-w", "100", "-t", "5", "-s", "-1"],
        },
        notes=[
            "logical_threads is -w: that many producers and that many consumers.",
            "os_threads is 2*logical_threads. The 2019 script's extra doubling is recorded and not applied.",
            "The paper point is -w 100 (100 producers and 100 consumers), outside the 96.",
            "A scaling report uses effective_worker_threads, not the matrix column.",
        ],
        logical_threads=logical_threads,
        workers=2 * logical_threads,
    )


def _redis(logical_threads: int) -> dict[str, Any]:
    client = [
        "{artifact:redis-benchmark}",
        "-r",
        "1000000",
        "-n",
        str(REDIS_REQUESTS),
        "-P",
        str(REDIS_PIPELINE_DEPTH),
        "-q",
        "lpush",
        "a",
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
        "10",
        "lrange",
        "a",
        "1",
        "10",
    ]
    return {
        "measurement_subject": "server",
        "thread_effect": "none",
        "argv": None,
        "server_argv": ["{artifact:redis-server}"],
        "client_argv": client,
        "parameter_roles": {
            "logical_threads_controls_argv": False,
            "logical_threads_stored_only": logical_threads,
            "pipeline_depth": REDIS_PIPELINE_DEPTH,
            "pipeline_depth_is_not_logical_threads": True,
            "requests": REDIS_REQUESTS,
            "key": "a",
        },
        "metrics": {
            "throughput": {"subject": "client", "window": "request-window"},
            "wall_time": {"subject": "client", "window": "request-window"},
            "perf": {"subject": "server", "window": "request-window"},
            "peak_rss": {"subject": "server", "window": "lifetime"},
        },
        "concurrency": _concurrency(logical_threads, None, False, None),
        "lifecycle": [
            "spawn_server_with_preload",
            "wait_until_redis_cli_ping_pongs",
            "verify_allocator_on_server_pid",
            "begin_request_window_perf_on_server",
            "run_client_one_million_requests",
            "end_request_window_perf",
            "redis_cli_shutdown",
            "wait_server_exit",
            "parse_client_request_window",
            "parse_server_lifetime_peak_rss",
        ],
        "notes": [
            "logical_threads does not change the server or the client argv.",
            "This is not a scaling row. Six matrix keys share one execution.",
            "-P 8 is pipeline depth from bench.sh. It is not the matrix thread column.",
            "throughput and wall_time are the client request window. perf is the server during that same window. peak RSS is the server's whole lifetime, not the client's.",
            "bench.sh slept 2 seconds instead of waiting for PING. This lifecycle waits for PONG.",
        ],
    }
