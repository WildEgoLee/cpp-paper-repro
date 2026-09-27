"""Start a workload only after the parent knows its pid.

The child stops before exec. The parent attaches measurement, then releases
the child. The workload body cannot run before that release.
"""

from __future__ import annotations

import os
import signal
import time
from pathlib import Path
from typing import Any, Callable


class LaunchResult(dict):
    pass


def launch_foreground(
    argv: list[str],
    env: dict[str, str],
    stdout_path: Path,
    stderr_path: Path,
    while_stopped: Callable[[int], None],
    after_exec: Callable[[int, str], None],
    ready: Callable[[str], bool] | None = None,
) -> dict[str, Any]:
    stdout_path.parent.mkdir(parents=True, exist_ok=True)
    out_fd = os.open(stdout_path, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o644)
    err_fd = os.open(stderr_path, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o644)
    pid = os.fork()
    if pid == 0:
        try:
            os.dup2(out_fd, 1)
            os.dup2(err_fd, 2)
            os.closerange(3, 64)
            os.kill(os.getpid(), signal.SIGSTOP)
            os.execvpe(argv[0], argv, env)
        finally:
            os._exit(127)
    os.close(out_fd)
    os.close(err_fd)
    return _parent(pid, argv, while_stopped, after_exec, ready)


def spawn_exec(
    argv: list[str],
    env: dict[str, str],
    stdout_path: Path,
    stderr_path: Path,
) -> int:
    """Direct child, no shell. Used for the Redis server."""
    stdout_path.parent.mkdir(parents=True, exist_ok=True)
    out_fd = os.open(stdout_path, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o644)
    err_fd = os.open(stderr_path, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o644)
    pid = os.fork()
    if pid == 0:
        try:
            os.dup2(out_fd, 1)
            os.dup2(err_fd, 2)
            os.closerange(3, 64)
            os.execvpe(argv[0], argv, env)
        finally:
            os._exit(127)
    os.close(out_fd)
    os.close(err_fd)
    return pid


def _parent(
    pid: int,
    argv: list[str],
    while_stopped: Callable[[int], None],
    after_exec: Callable[[int, str], None],
    ready: Callable[[str], bool] | None,
) -> dict[str, Any]:
    trace = ["child_forked"]
    stopped, status = os.waitpid(pid, os.WUNTRACED)
    if stopped != pid or not os.WIFSTOPPED(status):
        os.kill(pid, signal.SIGKILL)
        os.waitpid(pid, 0)
        return {"pid": pid, "exit_code": 127, "trace": trace + ["not_stopped"], "rusage": None, "wall_time_s": None}
    trace.append("child_stopped")
    while_stopped(pid)
    trace.append("measurement_ready")
    started = time.perf_counter()
    os.kill(pid, signal.SIGCONT)
    trace.append("child_released")
    maps_text, saw_exec = _wait_until_exec(pid, argv[0], ready)
    trace.append("exec_observed" if saw_exec else "exec_missed")
    after_exec(pid, maps_text)
    trace.append("maps_checked")
    waited, wait_status, usage = os.wait4(pid, 0)
    wall = time.perf_counter() - started
    if os.WIFEXITED(wait_status):
        code = os.WEXITSTATUS(wait_status)
    else:
        code = 128 + os.WTERMSIG(wait_status)
    return {
        "pid": pid,
        "exit_code": code,
        "trace": trace,
        "rusage": usage,
        "wall_time_s": wall,
        "maps_text": maps_text,
        "saw_exec": saw_exec,
        "waited": waited,
    }


def _wait_until_exec(
    pid: int,
    argv0: str,
    ready: Callable[[str], bool] | None,
    timeout: float = 2.0,
) -> tuple[str, bool]:
    """Wait until exe is the target and its mappings look loaded.

    /proc/pid/exe flips to the new binary while ld.so is still mapping
    libc and LD_PRELOAD. A maps snapshot at that instant misses them.
    """
    target = str(Path(argv0).resolve()) if Path(argv0).exists() else argv0
    deadline = time.monotonic() + timeout
    last = ""
    while time.monotonic() < deadline:
        if not Path(f"/proc/{pid}").exists():
            return last, False
        try:
            exe = os.readlink(f"/proc/{pid}/exe")
        except OSError:
            return last, False
        try:
            resolved = str(Path(exe).resolve())
        except OSError:
            resolved = exe
        if resolved == target:
            try:
                last = Path(f"/proc/{pid}/maps").read_text(errors="replace")
            except OSError:
                return last, False
            if ready is None or ready(last):
                return last, True
        time.sleep(0.01)
    return last, False
