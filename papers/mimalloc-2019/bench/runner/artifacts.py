"""Declared artifacts. Paths stay unresolved until a later execution commit.

Dry-run writes this file so a plan names binaries without hashing ones that
are not on the machine. Missing paths are status unresolved, not invented.
"""

from __future__ import annotations

import hashlib
from pathlib import Path
from typing import Any

from . import SCHEMA_ARTIFACTS


ALLOCATOR_LIBS = {
    "glibc": None,
    "mimalloc-v1.0.0": "libmimalloc.so",
    "jemalloc": "libjemalloc.so",
    "tcmalloc": "libtcmalloc.so",
}

BENCHMARKS = [
    "alloc-test",
    "larson",
    "xmalloc-test",
    "redis-server",
    "redis-benchmark",
    "redis-cli",
]


def unresolved_manifest() -> dict[str, Any]:
    allocators = {}
    for name, filename in ALLOCATOR_LIBS.items():
        allocators[name] = {
            "expected_basename": filename,
            "path": None,
            "sha256": None,
            "status": "system" if name == "glibc" else "unresolved",
        }
    benchmarks = {
        name: {"path": None, "sha256": None, "status": "unresolved"} for name in BENCHMARKS
    }
    return {"schema": SCHEMA_ARTIFACTS, "allocators": allocators, "benchmarks": benchmarks}


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()
