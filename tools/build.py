#!/usr/bin/env python3
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


def main() -> int:
	ap = argparse.ArgumentParser()
	ap.add_argument("--dll", action="store_true", help="Build DLL variant")
	ap.add_argument("--toolchain", choices=["gcc", "clang", "msvc", "auto"], default="gcc")
	ap.add_argument("--config", default="Release", help="Build config for multi-config generators")
	ap.add_argument("--build-root", default="build", help="Root directory for build trees and outputs")
	ap.add_argument("--reset-build", action="store_true", help="Reset selected build tree before configure")
	args = ap.parse_args()

	root = Path(__file__).resolve().parents[1]
	build_root = (root / args.build_root).resolve()
	mode = "dll" if args.dll else "exe"
	build_dir = build_root / mode

	init_cmd = [
		sys.executable,
		str(root / "tools" / "init.py"),
		"--toolchain",
		args.toolchain,
		"--build-dir",
		str(build_dir),
		"--output-dir",
		str(build_root),
	]
	if args.dll:
		init_cmd.append("--dll")
	if args.reset_build:
		init_cmd.append("--reset-build")

	print("[build] running:", " ".join(init_cmd))
	rc = subprocess.run(init_cmd).returncode
	if rc != 0:
		return rc

	build_cmd = [
		"cmake",
		"--build",
		str(build_dir),
		"--config",
		args.config,
	]
	print("[build] running:", " ".join(build_cmd))
	return subprocess.run(build_cmd).returncode


if __name__ == "__main__":
	raise SystemExit(main())
