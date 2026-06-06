#!/usr/bin/env python3
"""
PUBLIC tests for the `hexdump` exercise.
Run from the project root: python3 test_hexdump_public.py
"""

import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from test_harness import build, run, TempFile, expect, print_summary

EXE = "./hexdump"

def main():
    print("Building hexdump...")
    if not build("hexdump"):
        sys.exit(1)
    print()
    print("Running public tests for hexdump")
    print("─" * 50)

    # ── Test 1: basic short file (from spec example 1) ────────────────────────
    with TempFile(b"Hello, world!") as path:
        rc, out, _ = run([EXE, path])
        lines = out.decode(errors="replace").splitlines()
        expect(rc == 0, "exit code 0 for valid file")
        expect(len(lines) == 1, "single line for 13-byte file",
               f"got {len(lines)} line(s)")
        if lines:
            expect("48 65 6C 6C 6F 2C 20 77" in lines[0],
                   "correct hex for first 8 bytes of 'Hello, world!'",
                   f"line: {lines[0]!r}")
            expect("Hello, world!" in lines[0],
                   "ASCII portion contains 'Hello, world!'",
                   f"line: {lines[0]!r}")

    # ── Test 2: missing file → stderr + exit code 1 ───────────────────────────
    rc, out, err = run([EXE, "this-file-does-not-exist-xyz"])
    expect(rc == 1, "exit code 1 for missing file",
           f"got exit code {rc}")
    combined = (out + err).decode(errors="replace")
    expect("hexdump" in combined,
           "error message contains 'hexdump' prefix",
           f"stderr/stdout: {combined!r}")

    # ── Test 3: no extra output on stdout for error case ──────────────────────
    expect(out.strip() == b"",
           "stdout is empty when file is missing",
           f"stdout was: {out!r}")

    passed, total = print_summary()
    sys.exit(0 if passed == total else 1)

if __name__ == "__main__":
    main()
