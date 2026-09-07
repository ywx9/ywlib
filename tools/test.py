from __future__ import annotations

import argparse
import re
import subprocess
import sys

from _common import project_root, read_text, run
from _project import load_project_config


def main() -> int:
    parser = argparse.ArgumentParser(description="Compile and link build/test.cpp.")
    parser.add_argument("--debug", action="store_true", help="use Debug settings")
    args = parser.parse_args()
    root = project_root()
    source_dir = root / "build"
    build_dir = source_dir / "test-build"
    if not (source_dir / "test.cpp").is_file():
        raise FileNotFoundError("Create build/test.cpp before running tools/test.py.")

    config = load_project_config(root / "project.json")
    cmake_text = read_text(root / "CMakeLists.txt")
    cmake_text, count = re.subn(
        r'(?<![^\s(\"])' + re.escape(config.source_file) + r'(?=[\s)\"])',
        "test.cpp",
        cmake_text,
    )
    if not count:
        raise ValueError("Source from project.json was not found in CMakeLists.txt.")
    cmake_text = cmake_text.replace(
        "${CMAKE_CURRENT_SOURCE_DIR}/ywlib",
        "${CMAKE_CURRENT_SOURCE_DIR}/../ywlib",
    )

    build_type = "Debug" if args.debug else "Release"
    command = [
        "cmake", "-S", str(source_dir), "-B", str(build_dir),
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
        f"-DCMAKE_BUILD_TYPE={build_type}",
    ]
    if not (build_dir / "CMakeCache.txt").is_file():
        command.extend(["-G", "MinGW Makefiles"])

    copied_cmake = source_dir / "CMakeLists.txt"
    # Exclusive creation preserves any pre-existing file at this path.
    with copied_cmake.open("x", encoding="utf-8", newline="\n") as output:
        try:
            output.write(cmake_text)
        except BaseException:
            output.close()
            copied_cmake.unlink()
            raise
    try:
        run(command, cwd=root)
        run(["cmake", "--build", str(build_dir), "--config", build_type], cwd=root)
    finally:
        copied_cmake.unlink()
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as e:
        raise SystemExit(e.returncode)
    except Exception as e:
        print(f"error: {e}", file=sys.stderr)
        raise SystemExit(1)
