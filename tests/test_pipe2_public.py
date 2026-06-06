#!/usr/bin/env python3
"""
PUBLIC tests for the `pipe2` exercise.
Run from the project root: python3 test_pipe2_public.py
"""

import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from test_harness import build, run, expect, print_summary

EXE = "./pipe2"

def main():
    print("Building pipe2...")
    if not build("pipe2"):
        sys.exit(1)
    print()
    print("Running public tests for pipe2")
    print("─" * 50)

    # ── Test 1: echo hello world | tr a-z A-Z ────────────────────────────────
    rc, out, _ = run([EXE, "echo hello world", "tr a-z A-Z"], timeout=10)
    text = out.decode(errors="replace").strip()
    expect(rc == 0, "tr-uppercase: exit 0", f"got {rc}")
    expect(text == "HELLO WORLD",
           "tr-uppercase: output is 'HELLO WORLD'",
           f"got {text!r}")

    # ── Test 2: exit code comes from right-hand command ───────────────────────
    rc, out, _ = run([EXE, "echo foo", "true"], timeout=10)
    expect(rc == 0, "exit-from-right: true gives exit 0", f"got {rc}")

    rc2, _, _ = run([EXE, "echo foo", "false"], timeout=10)
    expect(rc2 == 1, "exit-from-right: false gives exit 1", f"got {rc2}")

    passed, total = print_summary()
    sys.exit(0 if passed == total else 1)

if __name__ == "__main__":
    main()
