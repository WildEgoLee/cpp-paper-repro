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
from .adapters import resolve
from .validate import validate_resolved

BENCH_DIR = Path(__file__).resolve().parent.parent
MATRIX_PATH = BENCH_DIR / "matrix.json"

PROFILES = {
    "round1": {"warmup": 3, "measured": 15, "threads": None},
    "smoke": {"warmup": 1, "measured": 1, "threads": [1]},
}


def load_matrix(path: Path | None = None) -> dict[str, Any]:
    return json.loads((path or MATRIX_PATH).read_text())


def expand(profile: str, matrix: dict[str, Any] | None = None, seed: int = PLAN_SEED) -> dict[str, Any]:
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
                validate_resolved(command)
                for phase, count in (("warmup", warmup), ("measured", measured)):
                    for repetition in range(1, count + 1):
                        block.append(_invocation(command, phase, repetition))
            rng = random.Random(f"{seed}:{workload}:{logical_threads}")
            rng.shuffle(block)
            blocks.append(block)

    invocations = [item for block in blocks for item in block]
    configurations = len(allocators) * len(workloads) * len(threads)
    return {
        "schema": SCHEMA_PLAN,
        "profile": profile,
        "seed": seed,
        "shuffle": "inside each (workload, logical_threads) block; blocks stay in matrix order",
        "executes": False,
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
        "command": command,
    }


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
