from __future__ import annotations

import argparse
import subprocess
import sys

from _common import (
    cmake_configure,
    ensure_file_from_template,
    project_root,
    run,
    templates_dir,
)
from _project import load_project_config, project_template_values


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

    project_json_path = root / "project.json"
    if not project_json_path.is_file():
        print(
            "error: project.json is missing. Run: python tools/init.py first.",
            file=sys.stderr,
        )
        return 1

    cmake_lists_path = root / "CMakeLists.txt"
    if not cmake_lists_path.is_file():
        config = load_project_config(project_json_path)
        ensure_file_from_template(
            cmake_lists_path,
            templates_dir() / "CMakeLists.txt.in",
            project_template_values(config),
            overwrite=False,
        )

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
