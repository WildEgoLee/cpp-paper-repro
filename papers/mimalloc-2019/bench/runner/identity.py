"""A run is immutable once it is sealed.

plan.json, host.json, and artifacts.json are hashed. Artifact files are
hashed again on resume. A mismatch refuses the run instead of mixing it
with a later dry-run or a different binary.
"""

from __future__ import annotations

import json
import os
from pathlib import Path
from typing import Any

from .artifacts import file_sha256


class IdentityMismatch(RuntimeError):
    pass


class RunFrozen(RuntimeError):
    pass


def host_identity(host: dict[str, Any]) -> dict[str, Any]:
    return {
        "uname": host.get("uname"),
        "hardware_threads": host.get("hardware_threads"),
        "physical_cores": host.get("physical_cores"),
        "numa_nodes": host.get("numa_nodes"),
    }


def has_cases(run_dir: Path) -> bool:
    cases = run_dir / "cases"
    return cases.is_dir() and any(cases.rglob("*.json"))


def assert_dry_run_allowed(run_dir: Path) -> None:
    if (run_dir / "identity.json").exists() or has_cases(run_dir):
        raise RunFrozen(f"{run_dir} already has cases or a sealed identity")


def seal(run_dir: Path, repo_sha: str) -> dict[str, Any]:
    path = run_dir / "identity.json"
    if path.exists():
        verify(run_dir, repo_sha)
        return json.loads(path.read_text())
    if has_cases(run_dir):
        raise IdentityMismatch("cases exist without identity.json")
    artifacts = json.loads((run_dir / "artifacts.json").read_text())
    host = json.loads((run_dir / "host.json").read_text())
    ident = {
        "schema": "cpp-paper-repro.identity/v1",
        "repo_sha": repo_sha,
        "plan_sha256": file_sha256(run_dir / "plan.json"),
        "host_sha256": file_sha256(run_dir / "host.json"),
        "artifacts_sha256": file_sha256(run_dir / "artifacts.json"),
        "host_identity": host_identity(host),
        "artifact_files": artifact_file_hashes(artifacts),
    }
    _atomic(path, ident)
    return ident


def verify(run_dir: Path, repo_sha: str) -> None:
    ident = json.loads((run_dir / "identity.json").read_text())
    host = json.loads((run_dir / "host.json").read_text())
    artifacts = json.loads((run_dir / "artifacts.json").read_text())
    problems = []
    if ident.get("repo_sha") != repo_sha:
        problems.append("repo_sha")
    if file_sha256(run_dir / "plan.json") != ident.get("plan_sha256"):
        problems.append("plan")
    if file_sha256(run_dir / "host.json") != ident.get("host_sha256"):
        problems.append("host")
    if host_identity(host) != ident.get("host_identity"):
        problems.append("host_identity")
    if file_sha256(run_dir / "artifacts.json") != ident.get("artifacts_sha256"):
        problems.append("artifacts")
    if artifact_file_hashes(artifacts) != ident.get("artifact_files"):
        problems.append("artifact_files")
    if problems:
        raise IdentityMismatch("resume refused: " + ", ".join(problems))


def artifact_file_hashes(artifacts: dict[str, Any]) -> dict[str, str]:
    found: dict[str, str] = {}
    for section in ("allocators", "benchmarks"):
        for entry in artifacts.get(section, {}).values():
            for field in ("path", "loader"):
                raw = entry.get(field)
                if raw:
                    found[raw] = file_sha256(Path(raw))
    return found


def _atomic(path: Path, payload: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_name(path.name + ".tmp")
    tmp.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n")
    os.replace(tmp, path)
