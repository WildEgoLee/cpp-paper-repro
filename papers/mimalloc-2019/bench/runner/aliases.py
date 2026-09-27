"""Explicit ids. Unknown strings raise; nothing is guessed by prefix or edit distance.

Canonical allocator and workload ids are the strings in bench/matrix.json.
Paper and environment-lock spellings are aliases onto those, not a second matrix.
"""

from __future__ import annotations

ALLOCATOR_ALIASES = {
    "glibc": "glibc",
    "mi": "mimalloc-v1.0.0",
    "mimalloc": "mimalloc-v1.0.0",
    "mimalloc-v1.0.0": "mimalloc-v1.0.0",
    "je": "jemalloc",
    "jemalloc": "jemalloc",
    "tc": "tcmalloc",
    "tcmalloc": "tcmalloc",
}

WORKLOAD_ALIASES = {
    "alloc-test": "alloc-test",
    "larson": "larson",
    "xmalloc": "xmalloc",
    # environment-paper.json uses the benchmark's historical name.
    "xmalloc-test": "xmalloc",
    "redis": "redis",
}


class UnknownId(ValueError):
    pass


def canonical_allocator(name: str) -> str:
    try:
        return ALLOCATOR_ALIASES[name]
    except KeyError as exc:
        raise UnknownId(f"unknown allocator id {name!r}") from exc


def canonical_workload(name: str) -> str:
    try:
        return WORKLOAD_ALIASES[name]
    except KeyError as exc:
        raise UnknownId(f"unknown workload id {name!r}") from exc
