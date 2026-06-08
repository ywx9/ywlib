from __future__ import annotations

import argparse
import subprocess
import sys

from _common import cmake_configure, project_root, run


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--debug",
        action="store_true",
        help="configure and build with Debug settings",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    root = project_root()

    build_dir = root / "build"
    build_type = "Debug" if args.debug else "Release"
    cmake_configure(build_dir, build_type=build_type)

    run(
        [
            "cmake",
            "--build",
            str(build_dir),
        ],
        cwd=root,
    )

    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as e:
        raise SystemExit(e.returncode)
    except Exception as e:
        print(f"error: {e}", file=sys.stderr)
        raise SystemExit(1)
