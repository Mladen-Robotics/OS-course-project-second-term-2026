#!/usr/bin/env python3
"""
PUBLIC tests for the `prun` exercise.
Run from the project root: python3 test_prun_public.py
"""

import sys, os, re
sys.path.insert(0, os.path.dirname(__file__))
from test_harness import build, run, expect, print_summary

EXE = "./prun"

def main():
    print("Building prun...")
    if not build("prun"):
        sys.exit(1)
    print()
    print("Running public tests for prun")
    print("─" * 50)

    # ── Test 1: single command ────────────────────────────────────────────────
    rc, out, _ = run([EXE, "echo hello"], timeout=10)
    text = out.decode(errors="replace")
    expect(rc == 0, "single-cmd: exit 0", f"got {rc}")
    # Line must match: [<pid>] "echo hello" exited with status 0
    match = re.search(r'\[(\d+)\] "echo hello" exited with status 0', text)
    expect(match is not None,
           'single-cmd: output line matches [<pid>] "echo hello" exited with status 0',
           f"output: {text!r}")

    # ── Test 2: two commands, both reported ───────────────────────────────────
    rc, out, _ = run([EXE, "echo hello", "echo world"], timeout=10)
    text = out.decode(errors="replace")
    expect(rc == 0, "two-cmds: exit 0", f"got {rc}")
    lines = [l for l in text.splitlines() if l.strip()]
    expect(len(lines) == 2, "two-cmds: exactly 2 output lines",
           f"lines: {lines}")

    # ── Test 3: failed command exit code reported correctly ───────────────────
    rc, out, _ = run([EXE, "ls /nonexistent-path-xyz"], timeout=10)
    text = out.decode(errors="replace")
    # ls returns non-zero for nonexistent paths; code should not be 0
    match = re.search(r'exited with status (\d+)', text)
    expect(match is not None and match.group(1) != "0",
           "failing-cmd: non-zero exit status reported",
           f"output: {text!r}")

    passed, total = print_summary()
    sys.exit(0 if passed == total else 1)

if __name__ == "__main__":
    main()
