"""preflight, dry-run, and execute. Execute refuses an unsealed or drifted run."""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

from .artifacts import unresolved_manifest
from .execute import execute_run
from .identity import IdentityMismatch, RunFrozen, assert_dry_run_allowed
from .plan import PROFILES, expand
from .preflight import read_host


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="runner")
    sub = parser.add_subparsers(dest="cmd", required=True)

    pre = sub.add_parser("preflight", help="read host.json fields and round-1 eligibility")
    pre.add_argument("--strict", action="store_true", help="exit 2 when the host cannot run round-1")
    pre.add_argument("--out", type=Path)

    dry = sub.add_parser("dry-run", help="write a plan and do not execute it")
    dry.add_argument("--profile", choices=sorted(PROFILES), default="round1")
    dry.add_argument("--out", type=Path, required=True)

    exe = sub.add_parser("execute", help="run pending invocations in a sealed directory")
    exe.add_argument("--run", type=Path, required=True)
    exe.add_argument("--repo-sha", default=None)
    exe.add_argument("--limit", type=int, default=None)

    args = parser.parse_args(argv)
    if args.cmd == "preflight":
        host = read_host()
        _emit(host, args.out)
        if args.strict and not host["eligible_for_round_1"]:
            return 2
        return 0
    if args.cmd == "dry-run":
        try:
            assert_dry_run_allowed(args.out)
        except RunFrozen as exc:
            print(str(exc), file=sys.stderr)
            return 3
        host = read_host()
        plan = expand(args.profile, hardware_threads=host["hardware_threads"])
        out = args.out
        out.mkdir(parents=True, exist_ok=True)
        _emit(host, out / "host.json")
        _emit(unresolved_manifest(), out / "artifacts.json")
        _emit(plan, out / "plan.json")
        print(
            f"dry-run profile={args.profile} configurations={plan['configurations']} "
            f"invocations={plan['invocations']} eligible={host['eligible_for_round_1']} "
            f"out={out}"
        )
        return 0
    if args.cmd == "execute":
        try:
            result = execute_run(args.run, args.repo_sha or _repo_sha(), limit=args.limit)
        except (IdentityMismatch, RunFrozen, FileNotFoundError, RuntimeError) as exc:
            print(str(exc), file=sys.stderr)
            return 4
        print(json.dumps(result))
        return 0
    return 1


def _repo_sha() -> str:
    ran = subprocess.run(["git", "rev-parse", "HEAD"], check=False, capture_output=True, text=True)
    if ran.returncode != 0 or not ran.stdout.strip():
        raise RuntimeError("repo sha unavailable")
    return ran.stdout.strip()


def _emit(payload: dict, path: Path | None) -> None:
    text = json.dumps(payload, indent=2, sort_keys=True) + "\n"
    if path is None:
        sys.stdout.write(text)
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_name(path.name + ".tmp")
    tmp.write_text(text)
    tmp.replace(path)


if __name__ == "__main__":
    raise SystemExit(main())
