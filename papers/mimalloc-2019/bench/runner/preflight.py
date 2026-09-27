"""Read the host. Do not invent missing sysfs values.

Round-1 still requires 12 hardware threads. That gate covers alloc-test and
larson, whose worker count equals the matrix column. It does not mean an
xmalloc column of 12 fits: that row starts 24 workers. Redis is not a
scaling row at all. Per-invocation oversubscribed is computed later from
effective_worker_threads, not by raising this gate to 24.
"""

from __future__ import annotations

import os
import platform
from pathlib import Path
from typing import Any

from . import SCHEMA_HOST

MIN_HARDWARE_THREADS = 12


def read_host() -> dict[str, Any]:
    threads = _hardware_threads()
    cores = _physical_cores()
    smt = None if threads is None or cores is None else threads > cores
    governor = _first_text(
        [
            Path("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"),
        ]
    )
    turbo = _turbo()
    numa = _numa_nodes()
    eligible = threads is not None and threads >= MIN_HARDWARE_THREADS
    reasons = []
    if threads is None:
        reasons.append("hardware thread count unavailable")
    elif threads < MIN_HARDWARE_THREADS:
        reasons.append(
            f"{threads} hardware threads < {MIN_HARDWARE_THREADS}; "
            "alloc-test and larson columns above that count would be oversubscribed. "
            "xmalloc oversubscription is recorded per invocation and does not move this gate."
        )
    return {
        "schema": SCHEMA_HOST,
        "eligible_for_round_1": eligible,
        "ineligible_reasons": reasons,
        "min_hardware_threads": MIN_HARDWARE_THREADS,
        "gate_scope": {
            "protects": ["alloc-test", "larson"],
            "xmalloc": "effective_worker_threads is 2*logical_threads; oversubscribed is per invocation",
            "redis": "not a scaling row",
            "does_not_mean": "logical_threads=12 fits on a 12-thread host for every workload",
        },
        "hardware_threads": threads,
        "physical_cores": cores,
        "smt": smt,
        "numa_nodes": numa,
        "scaling_governor": governor,
        "turbo": turbo,
        "uname": " ".join(platform.uname()),
        "python": platform.python_version(),
    }


def _hardware_threads() -> int | None:
    cpuinfo = Path("/proc/cpuinfo")
    if cpuinfo.exists():
        count = 0
        for line in cpuinfo.read_text(errors="replace").splitlines():
            if line.startswith("processor"):
                count += 1
        if count:
            return count
    count = os.cpu_count()
    return count if count else None


def _physical_cores() -> int | None:
    cpuinfo = Path("/proc/cpuinfo")
    if not cpuinfo.exists():
        return None
    packages: dict[tuple[str, str], None] = {}
    physical = None
    core = None
    seen = False
    for line in cpuinfo.read_text(errors="replace").splitlines():
        if not line.strip():
            if physical is not None and core is not None:
                packages[(physical, core)] = None
                seen = True
            physical = None
            core = None
            continue
        if line.startswith("physical id"):
            physical = line.split(":", 1)[1].strip()
        elif line.startswith("core id"):
            core = line.split(":", 1)[1].strip()
    if physical is not None and core is not None:
        packages[(physical, core)] = None
        seen = True
    if not seen:
        return None
    return len(packages)


def _numa_nodes() -> int | None:
    node = Path("/sys/devices/system/node")
    if not node.is_dir():
        return None
    nodes = [p for p in node.glob("node[0-9]*") if p.is_dir()]
    return len(nodes) if nodes else None


def _turbo() -> bool | None:
    no_turbo = Path("/sys/devices/system/cpu/intel_pstate/no_turbo")
    if no_turbo.exists():
        text = no_turbo.read_text().strip()
        if text in ("0", "1"):
            return text == "0"
    boost = Path("/sys/devices/system/cpu/cpufreq/boost")
    if boost.exists():
        text = boost.read_text().strip()
        if text in ("0", "1"):
            return text == "1"
    return None


def _first_text(paths: list[Path]) -> str | None:
    for path in paths:
        if path.exists():
            text = path.read_text().strip()
            return text or None
    return None
