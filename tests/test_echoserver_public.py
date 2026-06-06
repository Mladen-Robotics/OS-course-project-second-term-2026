#!/usr/bin/env python3
"""
PUBLIC tests for the `echoserver` / `echoclient` exercise.
Run from the project root: python3 test_echoserver_public.py
"""

import sys, os, time, socket, subprocess
sys.path.insert(0, os.path.dirname(__file__))
from test_harness import build, run, expect, print_summary

SERVER = "./echoserver"
CLIENT = "./echoclient"

def free_port() -> int:
    with socket.socket() as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]

def main():
    print("Building echoserver and echoclient...")
    if not build("echoserver") or not build("echoclient"):
        sys.exit(1)
    print()
    print("Running public tests for echoserver/echoclient")
    print("─" * 50)

    port = free_port()
    server = subprocess.Popen([SERVER, str(port)],
                              stdout=subprocess.DEVNULL,
                              stderr=subprocess.DEVNULL)
    time.sleep(0.2)

    try:
        # ── Test 1: basic echo ────────────────────────────────────────────────
        rc, out, _ = run([CLIENT, "127.0.0.1", str(port), "hello"], timeout=5)
        expect(rc == 0, "echo-hello: client exit 0", f"got {rc}")
        expect(out.decode(errors="replace").strip() == "hello",
               "echo-hello: echoed 'hello'",
               f"got {out!r}")

        # ── Test 2: second client reuses server ───────────────────────────────
        rc2, out2, _ = run([CLIENT, "127.0.0.1", str(port), "world"], timeout=5)
        expect(rc2 == 0, "echo-world: client exit 0", f"got {rc2}")
        expect(out2.decode(errors="replace").strip() == "world",
               "echo-world: echoed 'world'",
               f"got {out2!r}")
    finally:
        server.terminate()
        server.wait(timeout=3)

    passed, total = print_summary()
    sys.exit(0 if passed == total else 1)

if __name__ == "__main__":
    main()
