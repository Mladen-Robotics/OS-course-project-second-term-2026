#!/usr/bin/env python3
"""
PUBLIC tests for the `timeserver` / `timeclient` exercise.
Run from the project root: python3 test_timeserver_public.py
"""

import sys, os, re, time, subprocess
sys.path.insert(0, os.path.dirname(__file__))
from test_harness import build, run, expect, print_summary

SERVER = "./timeserver"
CLIENT = "./timeclient"
PORT   = "19876"

def main():
    print("Building timeserver and timeclient...")
    if not build("timeserver") or not build("timeclient"):
        sys.exit(1)
    print()
    print("Running public tests for timeserver/timeclient")
    print("─" * 50)

    # ── Test 1: basic round-trip ──────────────────────────────────────────────
    server = subprocess.Popen([SERVER, PORT],
                              stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(0.2)
    rc, out, _ = run([CLIENT, "127.0.0.1", PORT], timeout=5)
    server.wait(timeout=3)

    text = out.decode(errors="replace").strip()
    expect(rc == 0, "round-trip: client exit 0", f"got {rc}")
    # Format: YYYY-MM-DD HH:MM:SS
    match = re.match(r'^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}$', text)
    expect(match is not None,
           "round-trip: output matches YYYY-MM-DD HH:MM:SS",
           f"got {text!r}")

    # ── Test 2: connection refused → client exit 1 ────────────────────────────
    rc2, out2, err2 = run([CLIENT, "127.0.0.1", "19877"], timeout=5)
    expect(rc2 == 1, "conn-refused: client exits 1", f"got {rc2}")
    msg = (out2 + err2).decode(errors="replace")
    expect("timeclient" in msg,
           "conn-refused: error prefixed with 'timeclient'",
           f"output: {msg!r}")

    passed, total = print_summary()
    sys.exit(0 if passed == total else 1)

if __name__ == "__main__":
    main()
