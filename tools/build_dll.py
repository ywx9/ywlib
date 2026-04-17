#!/usr/bin/env python3
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

def main() -> int:
    workspace = Path(__file__).resolve().parents[1]
    cmd = [sys.executable, str(workspace / "tools" / "build.py"), "--dll", *sys.argv[1:]]
    print("[build_dll] deprecated. use: python tools/build.py --dll")
    return subprocess.run(cmd).returncode


if __name__ == "__main__":
    raise SystemExit(main())
