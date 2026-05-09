import argparse
import os
import re
import select
import signal
import subprocess
import sys
import time


ANSI_RE = re.compile(r"\x1b\[[0-9;]*[A-Za-z]")


def clean_line(line):
    return ANSI_RE.sub("", line).rstrip()


def stop_process(proc):
    if proc.poll() is not None:
        return

    try:
        os.killpg(proc.pid, signal.SIGTERM)
    except ProcessLookupError:
        return

    try:
        proc.wait(timeout=5)
    except subprocess.TimeoutExpired:
        os.killpg(proc.pid, signal.SIGKILL)
        proc.wait(timeout=5)


def main():
    parser = argparse.ArgumentParser(
        description="Run ESPHome hardware test until AP33772S startup logs are observed."
    )
    parser.add_argument("--esphome", default=".venv/bin/esphome")
    parser.add_argument("--config", default="examples/ap33772s-d1-mini.yaml")
    parser.add_argument("--device", default="/dev/ttyUSB0")
    parser.add_argument("--timeout", type=float, default=90.0)
    args = parser.parse_args()

    required_patterns = {
        "i2c scan found AP33772S": "Found device at address 0x52",
        "startup probe responded": "AP33772S responded:",
        "component dump detected AP33772S": "Detected: YES",
    }
    seen = {name: False for name in required_patterns}

    cmd = [args.esphome, "run", args.config, "--device", args.device, "--reset"]
    proc = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
        start_new_session=True,
    )

    deadline = time.monotonic() + args.timeout

    try:
        while True:
            if time.monotonic() > deadline:
                missing = ", ".join(name for name, found in seen.items() if not found)
                print(f"Timed out waiting for startup logs; missing: {missing}", file=sys.stderr)
                return 1

            ready, _, _ = select.select([proc.stdout], [], [], 0.1)
            if ready:
                line = proc.stdout.readline()
                if line:
                    line = clean_line(line)
                    print(line, flush=True)

                    for name, pattern in required_patterns.items():
                        if pattern in line:
                            seen[name] = True

                    if all(seen.values()):
                        print("AP33772S hardware smoke test passed; startup logs observed.", flush=True)
                        return 0

            if proc.poll() is not None:
                missing = ", ".join(name for name, found in seen.items() if not found)
                print(
                    f"ESPHome exited before startup logs completed; missing: {missing}",
                    file=sys.stderr,
                )
                return proc.returncode or 1
    finally:
        stop_process(proc)


if __name__ == "__main__":
    sys.exit(main())
