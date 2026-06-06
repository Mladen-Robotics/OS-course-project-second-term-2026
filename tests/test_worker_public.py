#!/usr/bin/env python3
"""
PUBLIC tests for the `worker` exercise.
Run from the project root: python3 test_worker_public.py
"""

import sys, os, signal, time, subprocess
sys.path.insert(0, os.path.dirname(__file__))
from test_harness import build, expect, print_summary

EXE = "./worker"

def main():
    print("Building worker...")
    if not build("worker"):
        sys.exit(1)
    print()
    print("Running public tests for worker")
    print("─" * 50)

    # ── Test 1: ticks are emitted and SIGINT causes clean shutdown ────────────
    proc = subprocess.Popen(
        [EXE],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    time.sleep(2.5)
    proc.send_signal(signal.SIGINT)
    try:
        proc.wait(timeout=3)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.wait()
        expect(False, "SIGINT: process did not exit within 3s of SIGINT")
        print_summary()
        sys.exit(1)

    out = proc.stdout.read().decode(errors="replace")
    lines = [l.strip() for l in out.splitlines() if l.strip()]

    tick_lines = [l for l in lines if l.startswith("tick ")]
    expect(len(tick_lines) >= 2,
           f"ticks: at least 2 'tick N' lines emitted (got {len(tick_lines)})",
           f"output:\n{out!r}")

    # Check tick numbering starts at 1 and is monotone
    if tick_lines:
        expect(tick_lines[0] == "tick 1",
               "ticks: first line is 'tick 1'",
               f"got {tick_lines[0]!r}")

    shutdown_lines = [l for l in lines if l.startswith("shutdown after")]
    expect(len(shutdown_lines) == 1,
           "shutdown: exactly one 'shutdown after N ticks' line",
           f"lines: {shutdown_lines}")

    expect(proc.returncode == 0,
           "SIGINT: exit code 0 after SIGINT",
           f"got {proc.returncode}")

    passed, total = print_summary()
    sys.exit(0 if passed == total else 1)

if __name__ == "__main__":
    main()
