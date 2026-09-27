"""One invocation is the resume unit. A configuration is not.

A case is complete only when exit_code is 0, the metric parser succeeded,
and the allocator check succeeded. The complete flag in the file is not
trusted on its own. Writes land on a temporary sibling and then rename.
"""

from __future__ import annotations

import json
import os
from pathlib import Path
from typing import Any

from . import SCHEMA_CASE
from .plan import case_path, invocation_id, scheduled_invocations


def is_complete(record: dict[str, Any]) -> bool:
    if record.get("schema") != SCHEMA_CASE:
        return False
    if record.get("exit_code") != 0:
        return False
    metrics = record.get("metrics") or {}
    check = record.get("allocator_check") or {}
    if metrics.get("parsed") is not True or check.get("ok") is not True:
        return False
    perf = metrics.get("perf")
    if isinstance(perf, dict) and perf.get("session") == "error":
        return False
    coverage = record.get("metric_coverage") or {}
    for name in ("wall_time", "peak_rss"):
        if coverage.get(name) in ("parse-error", "error"):
            return False
    return True


def write_case(run_dir: Path, record: dict[str, Any]) -> Path:
    key = record["key"]
    path = case_path(run_dir, key)
    path.parent.mkdir(parents=True, exist_ok=True)
    payload = dict(record)
    payload["schema"] = SCHEMA_CASE
    payload["id"] = invocation_id(key)
    payload["complete"] = is_complete(payload)
    tmp = path.with_name(path.name + ".tmp")
    tmp.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n")
    os.replace(tmp, path)
    return path


def read_case(path: Path) -> dict[str, Any] | None:
    if not path.exists():
        return None
    return json.loads(path.read_text())


def pending(plan: dict[str, Any], run_dir: Path) -> list[dict[str, Any]]:
    """Invocations that still need to run. Completed ones are skipped."""
    left = []
    for item in scheduled_invocations(plan):
        path = case_path(run_dir, item["key"])
        record = read_case(path)
        if record is None or not is_complete(record):
            left.append(item)
    return left
