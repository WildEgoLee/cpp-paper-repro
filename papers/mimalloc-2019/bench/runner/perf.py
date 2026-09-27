"""perf stat attached to a known pid. Counters stay inheriting.

--no-inherit is not used: allocator workloads create threads after exec.
A missing perf binary or a paranoid kernel is per-metric `unavailable`.
`<not supported>` is per-metric `unsupported`. Neither fails the parser.
"""

from __future__ import annotations

import shutil
import signal
import subprocess
from pathlib import Path
from typing import Any

PERF_EVENTS = (
    "cycles",
    "instructions",
    "cache-references",
    "cache-misses",
    "L1-dcache-load-misses",
    "LLC-load-misses",
)


def perf_argv(pid: int, out_path: Path) -> list[str]:
    argv = [
        "perf",
        "stat",
        "-x",
        ";",
        "-p",
        str(pid),
        "-e",
        ",".join(PERF_EVENTS),
        "-o",
        str(out_path),
    ]
    if "--no-inherit" in argv:
        raise RuntimeError("perf must follow threads created after exec")
    return argv


class PerfSession:
    def __init__(self, out_path: Path):
        self.out_path = out_path
        self.proc: subprocess.Popen[bytes] | None = None
        self.status = "unavailable"
        self.reason = ""

    def attach(self, pid: int) -> None:
        if shutil.which("perf") is None:
            self.status = "unavailable"
            self.reason = "perf not on PATH"
            return
        self.out_path.parent.mkdir(parents=True, exist_ok=True)
        self.proc = subprocess.Popen(
            perf_argv(pid, self.out_path),
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
        )
        try:
            self.proc.wait(timeout=0.4)
        except subprocess.TimeoutExpired:
            self.status = "ok"
            self.reason = ""
            return
        err = ""
        if self.proc.stderr is not None:
            err = self.proc.stderr.read().decode(errors="replace")
        self.reason = err[-500:]
        self.proc = None
        lowered = err.lower()
        if "paranoid" in lowered or "permission" in lowered or "access" in lowered or "not supported" in lowered:
            self.status = "unavailable"
        else:
            self.status = "error"

    def stop(self) -> str:
        if self.proc is not None:
            self.proc.send_signal(signal.SIGINT)
            try:
                self.proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait(timeout=2)
        if self.out_path.exists():
            return self.out_path.read_text(errors="replace")
        return ""


def parse_perf_stat(text: str) -> dict[str, Any]:
    """Parse `perf stat -x ;`. Empty input is not a successful parse."""
    coverage: dict[str, str] = {}
    values: dict[str, str] = {}
    recognized = 0
    for raw_line in text.splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        fields = line.split(";")
        event = _event_name(fields)
        if event is None:
            continue
        recognized += 1
        status = _value_status(fields[0])
        coverage[event] = status
        values[event] = fields[0].strip()
    if recognized == 0:
        return {"parsed": False, "coverage": {}, "values": {}, "reason": "no perf rows"}
    parsed = all(status != "parse-error" for status in coverage.values())
    return {"parsed": parsed, "coverage": coverage, "values": values, "reason": ""}


def coverage_from_session(session_status: str, text: str) -> dict[str, Any]:
    if session_status == "unavailable":
        return {
            "parsed": True,
            "coverage": {name: "unavailable" for name in PERF_EVENTS},
            "values": {},
            "reason": "perf unavailable",
        }
    if session_status == "error":
        parsed = parse_perf_stat(text)
        parsed["parsed"] = False
        parsed["reason"] = parsed.get("reason") or "perf session error"
        return parsed
    return parse_perf_stat(text)


def _event_name(fields: list[str]) -> str | None:
    for field in fields:
        name = field.strip()
        if name in PERF_EVENTS:
            return name
    return None


def _value_status(raw: str) -> str:
    token = raw.strip()
    if token in ("<not supported>", "not supported"):
        return "unsupported"
    if token in ("<not counted>", "<not available>", ""):
        return "unavailable"
    try:
        float(token)
    except ValueError:
        return "parse-error"
    return "ok"
