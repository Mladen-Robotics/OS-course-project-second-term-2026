#!/usr/bin/env python3
"""
PUBLIC tests for the `reverse` exercise.
Run from the project root: python3 test_reverse_public.py
"""

import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from test_harness import build, run, TempFile, expect, print_summary

EXE = "./reverse"

def reverse_file(content: bytes) -> tuple[int, bytes, bytes, bytes]:
    """Write content to a temp file, run reverse on it, return (rc, stdout, stderr, file_content_after)."""
    with TempFile(content) as path:
        rc, out, err = run([EXE, path])
        with open(path, "rb") as f:
            after = f.read()
    return rc, out, err, after

def main():
    print("Building reverse...")
    if not build("reverse"):
        sys.exit(1)
    print()
    print("Running public tests for reverse")
    print("─" * 50)

    # ── Test 1: abcdef → fedcba ───────────────────────────────────────────────
    with TempFile(b"abcdef") as path:
        rc, _, _, = run([EXE, path])[:3]
        with open(path, "rb") as f:
            after = f.read()
    expect(rc == 0, "abcdef: exit 0")
    expect(after == b"fedcba", "abcdef: file reversed in place",
           f"got {after!r}")

    # ── Test 2: Hello, world! ─────────────────────────────────────────────────
    with TempFile(b"Hello, world!") as path:
        rc, _, _ = run([EXE, path])[:3]
        with open(path, "rb") as f:
            after = f.read()
    expect(rc == 0, "hello-world: exit 0")
    expect(after == b"!dlrow ,olleH", "hello-world: file reversed in place",
           f"got {after!r}")

    # ── Test 3: missing file → exit 1 ────────────────────────────────────────
    rc, out, err = run([EXE, "no-such-file-xyz"])
    expect(rc == 1, "missing-file: exit 1", f"got {rc}")
    msg = (out + err).decode(errors="replace")
    expect("reverse" in msg, "missing-file: error prefixed with 'reverse'",
           f"output: {msg!r}")

    passed, total = print_summary()
    sys.exit(0 if passed == total else 1)

if __name__ == "__main__":
    main()
