"""Two reductions. Only protocol_statistics is the acceptance number.

paper_view can be computed from the same raw samples later. It is a different
function on purpose, so a 5-run average cannot be stored in place of the
median/p95 of the 15 measured samples.
"""

from __future__ import annotations

import math
from typing import Any


def protocol_statistics(measured_samples: list[float]) -> dict[str, Any]:
    if not measured_samples:
        raise ValueError("protocol_statistics needs the measured samples")
    ordered = sorted(measured_samples)
    return {
        "name": "protocol_statistics",
        "n": len(ordered),
        "warmup_excluded": True,
        "median": _median(ordered),
        "p95": _nearest_rank(ordered, 0.95),
    }


def paper_view(samples: list[float]) -> dict[str, Any]:
    """Average, the paper's reduction. Not the acceptance statistic."""
    if len(samples) != 5:
        raise ValueError("paper_view is defined on 5 samples, matching the paper")
    return {
        "name": "paper_view",
        "n": 5,
        "average": sum(samples) / 5.0,
        "acceptance": False,
    }


def _median(ordered: list[float]) -> float:
    n = len(ordered)
    mid = n // 2
    if n % 2:
        return float(ordered[mid])
    return (ordered[mid - 1] + ordered[mid]) / 2.0


def _nearest_rank(ordered: list[float], fraction: float) -> float:
    # ceil(0.95 * n) is the 1-based rank. n=15, p95 -> rank 15, index 14.
    n = len(ordered)
    rank = max(1, math.ceil(fraction * n))
    return float(ordered[rank - 1])
