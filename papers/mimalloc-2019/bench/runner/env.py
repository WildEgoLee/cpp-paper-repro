"""Child environments do not inherit an allocator choice.

LD_PRELOAD from the parent, a shell profile, or the caller is removed.
Only the preload named by the resolved plan is put back. glibc puts none.
"""

from __future__ import annotations

from typing import Any

STRIPPED = ("LD_PRELOAD", "DYLD_INSERT_LIBRARIES")


def child_env(parent: dict[str, str], planned_preload: str | None) -> tuple[dict[str, str], dict[str, Any]]:
    env = dict(parent)
    removed = {}
    for key in STRIPPED:
        if key in env:
            removed[key] = env.pop(key)
    applied: dict[str, str] = {}
    if planned_preload:
        env["LD_PRELOAD"] = planned_preload
        applied["LD_PRELOAD"] = planned_preload
    return env, {"removed": removed, "applied": applied}
