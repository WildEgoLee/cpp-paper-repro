"""Expand the frozen matrix into an immutable invocation list.

Shuffle only inside a (workload, logical_threads) block, with PLAN_SEED.
Block order follows matrix.json. The same seed reproduces the same plan.
"""

from __future__ import annotations

import json
import random
from pathlib import Path
from typing import Any

from . import PLAN_SEED, SCHEMA_PLAN
from .adapters import resolve, stamp_oversubscription
from .validate import validate_resolved

BENCH_DIR = Path(__file__).resolve().parent.parent
MATRIX_PATH = BENCH_DIR / "matrix.json"

PROFILES = {
    "round1": {"warmup": 3, "measured": 15, "threads": None},
    "smoke": {"warmup": 1, "measured": 1, "threads": [1]},
}


def load_matrix(path: Path | None = None) -> dict[str, Any]:
    return json.loads((path or MATRIX_PATH).read_text())


def expand(
    profile: str,
    matrix: dict[str, Any] | None = None,
    seed: int = PLAN_SEED,
    hardware_threads: int | None = None,
) -> dict[str, Any]:
    if profile not in PROFILES:
        raise ValueError(f"unknown profile {profile!r}")
    spec = PROFILES[profile]
    matrix = matrix if matrix is not None else load_matrix()
    allocators = list(matrix["allocators"])
    workloads = [item["id"] if isinstance(item, dict) else item for item in matrix["workloads"]]
    threads = list(spec["threads"] if spec["threads"] is not None else matrix["threads"])
    warmup = spec["warmup"]
    measured = spec["measured"]

    blocks: list[list[dict[str, Any]]] = []
    for workload in workloads:
        for logical_threads in threads:
            block = []
            for allocator in allocators:
                command = resolve(workload, int(logical_threads), allocator)
                stamp_oversubscription(command, hardware_threads)
                validate_resolved(command)
                for phase, count in (("warmup", warmup), ("measured", measured)):
                    for repetition in range(1, count + 1):
                        block.append(_invocation(command, phase, repetition))
            rng = random.Random(f"{seed}:{workload}:{logical_threads}")
            rng.shuffle(block)
            blocks.append(block)

    invocations = [item for block in blocks for item in block]
    configurations = len(allocators) * len(workloads) * len(threads)
    equivalence = {item["execution_equivalence_key"] for item in invocations}
    return {
        "schema": SCHEMA_PLAN,
        "profile": profile,
        "seed": seed,
        "shuffle": "inside each (workload, logical_threads) block; blocks stay in matrix order",
        "executes": False,
        "hardware_threads_at_plan": hardware_threads,
        "windows": {
            "lifetime": "process exec through process exit, including startup",
            "request-window": "client request start through client completion; excludes server startup and shutdown",
        },
        "analysis": {
            "matrix_column_is_not_os_threads": True,
            "xmalloc_scaling_axis": "effective_worker_threads",
            "redis_rows_are_not_scaling_evidence": True,
            "group_identical_executions_by": "execution_equivalence_key",
        },
        "acceptance_statistics": "protocol_statistics",
        "protocol_statistics": {
            "sample": "measured repetitions only",
            "warmup_excluded": True,
            "reduce": ["median", "p95"],
        },
        "paper_statistics_not_acceptance": {
            "aggregation": "average of 5 runs",
            "metrics": ["wall_clock", "peak_rss"],
            "note": "Generable later from raw samples. Must not replace protocol_statistics.",
        },
        "configurations": configurations,
        "execution_equivalence_keys": len(equivalence),
        "execution_equivalence_key_policy": {
            "analysis_only": True,
            "deduplicate_execution": False,
            "resume_identity": "invocation_id",
        },
        "invocations": len(invocations),
        "invocation_list": invocations,
    }


def _invocation(command: dict[str, Any], phase: str, repetition: int) -> dict[str, Any]:
    key = {
        "workload": command["workload"],
        "allocator": command["allocator"],
        "logical_threads": command["logical_threads"],
        "phase": phase,
        "repetition": repetition,
    }
    return {
        "key": key,
        "id": invocation_id(key),
        "resolved_configuration_id": _configuration_id(command),
        "execution_equivalence_key": _equivalence_key(command),
        "command": command,
    }


def _configuration_id(command: dict[str, Any]) -> str:
    return "{workload}/{allocator}/logical={logical_threads}".format(**command)


def _equivalence_key(command: dict[str, Any]) -> str:
    concurrency = command["concurrency"]
    if not concurrency["scaling_row"]:
        return "{workload}/{allocator}/not-a-scaling-row".format(**command)
    return "{workload}/{allocator}/workers={workers}".format(
        workers=concurrency["effective_worker_threads"],
        **command,
    )


def scheduled_invocations(plan: dict[str, Any]) -> list[dict[str, Any]]:
    """Every invocation, including Redis rows that share an equivalence key.

    The equivalence key is for analysis. It must not be used to skip work.
    """
    policy = plan.get("execution_equivalence_key_policy") or {}
    if policy.get("analysis_only") is not True or policy.get("deduplicate_execution") is not False:
        raise RuntimeError("execution_equivalence_key is analysis-only and must not deduplicate")
    if policy.get("resume_identity") != "invocation_id":
        raise RuntimeError("resume identity must be the invocation id")
    return list(plan["invocation_list"])


def invocation_id(key: dict[str, Any]) -> str:
    return "{workload}/{allocator}/{logical_threads}/{phase}/{repetition}".format(**key)


def case_path(run_dir: Path, key: dict[str, Any]) -> Path:
    return (
        run_dir
        / "cases"
        / key["workload"]
        / key["allocator"]
        / f"t{key['logical_threads']}"
        / key["phase"]
        / f"{int(key['repetition']):02d}.json"
    )
