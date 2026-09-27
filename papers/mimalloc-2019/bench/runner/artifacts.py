"""Declared artifacts. Paths stay unresolved until a later execution commit.

Dry-run writes this file so a plan names binaries without hashing ones that
are not on the machine. Missing paths are status unresolved, not invented.
"""

from __future__ import annotations

import hashlib
from pathlib import Path
import re
import subprocess
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


class UnresolvedArtifact(RuntimeError):
    pass


def describe_file(path: Path, *, status: str = "resolved") -> dict[str, Any]:
    resolved = path.resolve()
    return {"path": str(resolved), "sha256": file_sha256(resolved), "status": status}


def bind_manifest(
    benchmarks: dict[str, Path],
    libraries: dict[str, Path],
    glibc: dict[str, Any],
) -> dict[str, Any]:
    manifest = unresolved_manifest()
    for name, path in benchmarks.items():
        if name not in manifest["benchmarks"]:
            raise UnresolvedArtifact(name)
        manifest["benchmarks"][name].update(describe_file(path))
    for name, path in libraries.items():
        if name == "glibc" or name not in manifest["allocators"]:
            raise UnresolvedArtifact(name)
        manifest["allocators"][name].update(describe_file(path))
        manifest["allocators"][name]["expected_basename"] = path.name
    manifest["allocators"]["glibc"] = {
        "expected_basename": "libc.so.6",
        "path": glibc["path"],
        "sha256": glibc["sha256"],
        "version": glibc["version"],
        "loader": glibc.get("loader"),
        "loader_sha256": glibc.get("loader_sha256"),
        "status": "system",
        "ld_preload": None,
    }
    return manifest


def probe_glibc() -> dict[str, Any]:
    libc = _find_libc()
    loader = _find_loader()
    return {
        "path": str(libc.resolve()),
        "sha256": file_sha256(libc),
        "version": _libc_version(libc),
        "loader": None if loader is None else str(loader.resolve()),
        "loader_sha256": None if loader is None else file_sha256(loader),
    }


def lookup_token(token: str, artifacts: dict[str, Any]) -> str:
    if not (token.startswith("{") and token.endswith("}")):
        return token
    kind, _, name = token[1:-1].partition(":")
    section = "benchmarks" if kind == "artifact" else "allocators"
    if kind not in ("artifact", "lib") or name not in artifacts.get(section, {}):
        raise UnresolvedArtifact(token)
    path = artifacts[section][name].get("path")
    if not path:
        raise UnresolvedArtifact(token)
    return path


def substitute(argv: list[str] | None, artifacts: dict[str, Any]) -> list[str] | None:
    if argv is None:
        return None
    return [lookup_token(item, artifacts) for item in argv]


def _find_libc() -> Path:
    for candidate in (
        "/lib/x86_64-linux-gnu/libc.so.6",
        "/lib64/libc.so.6",
        "/usr/lib/x86_64-linux-gnu/libc.so.6",
        "/lib/aarch64-linux-gnu/libc.so.6",
    ):
        path = Path(candidate)
        if path.exists():
            return path
    listed = subprocess.run(["ldd", "/bin/true"], check=False, capture_output=True, text=True)
    for line in listed.stdout.splitlines():
        if "libc.so.6" in line and "=>" in line:
            return Path(line.split("=>", 1)[1].split()[0])
    raise UnresolvedArtifact("libc.so.6")


def _find_loader() -> Path | None:
    for candidate in (
        "/lib64/ld-linux-x86-64.so.2",
        "/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2",
        "/lib/aarch64-linux-gnu/ld-linux-aarch64.so.1",
    ):
        path = Path(candidate)
        if path.exists():
            return path
    return None


def _libc_version(libc: Path) -> str:
    ran = subprocess.run([str(libc)], check=False, capture_output=True, text=True)
    match = re.search(r"release version (\d+\.\d+(?:\.\d+)?)", ran.stdout + ran.stderr)
    if match:
        return match.group(1)
    conf = subprocess.run(["getconf", "GNU_LIBC_VERSION"], check=False, capture_output=True, text=True)
    match = re.search(r"(\d+\.\d+(?:\.\d+)?)", conf.stdout)
    if match:
        return match.group(1)
    raise UnresolvedArtifact("glibc version")
