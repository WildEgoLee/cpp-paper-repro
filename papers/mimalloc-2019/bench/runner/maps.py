"""Match /proc/pid/maps against the resolved library path, not its basename."""

from __future__ import annotations

from pathlib import Path
from typing import Any


def verify_maps_text(text: str, require_present: list[str], require_absent: list[str]) -> dict[str, Any]:
    lines = [line for line in text.splitlines() if line.strip()]
    matched: list[str] = []
    problems: list[dict[str, Any]] = []
    for path in require_present:
        hits = _hits(lines, path)
        if not hits:
            problems.append({"missing": path})
        else:
            matched.extend(hits)
    for path in require_absent:
        hits = _hits(lines, path)
        if hits:
            problems.append({"forbidden": path, "lines": hits})
    return {
        "ok": not problems,
        "method": "proc_maps",
        "matched_lines": matched,
        "problems": problems,
    }


def read_proc_maps(pid: int) -> str:
    path = Path(f"/proc/{pid}/maps")
    try:
        return path.read_text(errors="replace")
    except OSError as exc:
        return f"# unreadable: {exc}\n"


def _hits(lines: list[str], path: str) -> list[str]:
    real = path
    try:
        real = str(Path(path).resolve())
    except OSError:
        pass
    needles = {path, real}
    return [line for line in lines if any(needle and needle in line for needle in needles)]
