#!/usr/bin/env python3
from __future__ import annotations

import argparse
import platform
import shutil
import subprocess
import sys
from pathlib import Path

# ------------------------------------------------------------
# helpers
# ------------------------------------------------------------

def ensure_source_cpp(workspace: Path):
    source_cpp = workspace / "source.cpp"
    if source_cpp.exists():
        return
    template = workspace / "tools" / "templates" / "source.cpp.in"
    if not template.exists():
        raise RuntimeError("source.cpp template not found")
    print("[init_vscode] creating source.cpp from template")
    source_cpp.write_text(template.read_text(encoding="utf-8"), encoding="utf-8")

def ensure_vscode_settings(workspace: Path):
    vscode_dir = workspace / ".vscode"
    vscode_dir.mkdir(parents=True, exist_ok=True)
    settings = vscode_dir / "settings.json"
    if settings.exists():
        return None
    template = workspace / "tools" / "templates" / "settings.json.in"
    if not template.exists():
        raise RuntimeError("settings.json template not found")
    print("[init_vscode] creating .vscode/settings.json from template")
    settings.write_text(template.read_text(encoding="utf-8"), encoding="utf-8")
    return settings

def which_or_error(name: str) -> str:
    p = shutil.which(name)
    if not p:
        raise RuntimeError(f"{name} not found in PATH")
    return p

def pick_mingw_generator() -> str:
    return "Ninja" if shutil.which("ninja") else "MinGW Makefiles"

def run_cmake(root: Path, build: Path, extra: list[str]) -> int:
    cmd = [
        "cmake",
        "-S", str(root),
        "-B", str(build),
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
        *extra,
    ]
    print("[init_vscode] running:", " ".join(cmd))
    return subprocess.run(cmd).returncode

def ensure_cmakelists(workspace: Path):
    cmakelists = workspace / "CMakeLists.txt"
    if cmakelists.exists():
        return
    template = workspace / "tools" / "templates" / "CMakeLists.txt.in"
    if not template.exists():
        raise RuntimeError("CMakeLists.txt template not found")
    print("[init_vscode] creating CMakeLists.txt from template")
    cmakelists.write_text(template.read_text(encoding="utf-8"), encoding="utf-8")

def generate_wrapper_cmakelists(project_root: Path, generated_cmake: Path, kind: str) -> None:
    root_posix = project_root.resolve().as_posix()

    common = f'''cmake_minimum_required(VERSION 3.20)

set(YWLIB_PROJECT_ROOT "{root_posix}")

project(ywlib_wrapper LANGUAGES CXX)

add_subdirectory("${{YWLIB_PROJECT_ROOT}}" "${{CMAKE_BINARY_DIR}}/project_src")
'''

    if kind == "dll":
        body = r'''
add_library(${YOUR_APP_NAME} SHARED $<TARGET_OBJECTS:${YOUR_APP_NAME}_obj>)

target_link_libraries(${YOUR_APP_NAME}
    PRIVATE
        ${YOUR_APP_LIBRARIES}
)

set(YOUR_DLL_PREFIX "" CACHE STRING "Prefix for DLL/shared library filename")
set(YOUR_DLL_OUTPUT_NAME "${YOUR_APP_NAME}" CACHE STRING "Output filename base for DLL/shared library")

set_target_properties(${YOUR_APP_NAME} PROPERTIES
    PREFIX "${YOUR_DLL_PREFIX}"
    OUTPUT_NAME "${YOUR_DLL_OUTPUT_NAME}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}"
)
'''
    elif kind == "exe":
        body = r'''
if (WIN32 AND NOT SHOW_CONSOLE)
    add_executable(${YOUR_APP_NAME} WIN32 $<TARGET_OBJECTS:${YOUR_APP_NAME}_obj>)
else()
    add_executable(${YOUR_APP_NAME} $<TARGET_OBJECTS:${YOUR_APP_NAME}_obj>)
endif()

target_link_libraries(${YOUR_APP_NAME}
    PRIVATE
        ${YOUR_APP_LIBRARIES}
)

set_target_properties(${YOUR_APP_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)
'''
    else:
        raise ValueError(f"unknown wrapper kind: {kind}")

    generated_cmake.parent.mkdir(parents=True, exist_ok=True)
    generated_cmake.write_text(common + body, encoding="utf-8")
    print("[init_vscode] generated wrapper cmakelists:", generated_cmake)

# ------------------------------------------------------------
# main
# ------------------------------------------------------------

def main() -> int:
    workspace = Path(__file__).resolve().parents[1]
    ensure_source_cpp(workspace)
    ensure_cmakelists(workspace)

    ap = argparse.ArgumentParser()
    ap.add_argument(
        "--toolchain",
        choices=["gcc", "clang", "msvc", "auto"],
        default="gcc",
        help="Toolchain to use (default: gcc)",
    )
    ap.add_argument("--dll", action="store_true", help="Configure build as DLL")
    ap.add_argument("--build-dir", default="build")
    ap.add_argument("--output-dir", default=None, help="Directory for final binaries (.exe/.dll). Default: same as --build-dir")
    ap.add_argument("--reset-build", action="store_true")
    ap.add_argument("--reset-vscode", action="store_true")
    args = ap.parse_args()

    root = workspace
    build = root / args.build_dir
    output_dir = (root / args.output_dir) if args.output_dir else build
    vscode = root / ".vscode"

    if args.reset_build and build.exists():
        shutil.rmtree(build)
    if args.reset_vscode and vscode.exists():
        shutil.rmtree(vscode)

    build.mkdir(parents=True, exist_ok=True)
    output_dir.mkdir(parents=True, exist_ok=True)

    extra: list[str] = []

    try:
        if args.toolchain == "gcc":
            gen = pick_mingw_generator()
            gpp = which_or_error("g++")
            extra += [
                "-G", gen,
                f"-DCMAKE_CXX_COMPILER={gpp}",
            ]

        elif args.toolchain == "clang":
            clangpp = which_or_error("clang++")
            extra += [
                f"-DCMAKE_CXX_COMPILER={clangpp}",
            ]

        elif args.toolchain == "msvc":
            if platform.system().lower() != "windows":
                raise RuntimeError("msvc toolchain is only valid on Windows")
            extra += ["-G", "Visual Studio 17 2022"]

        elif args.toolchain == "auto":
            pass

    except RuntimeError as e:
        print(
            "[init_vscode] ERROR:", e,
            "\nHint: run from the appropriate environment "
            "(e.g. w64devkit shell for gcc).",
            file=sys.stderr,
        )
        return 2

    out = output_dir.resolve().as_posix()
    extra += [
        f"-DCMAKE_RUNTIME_OUTPUT_DIRECTORY={out}",
        f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={out}",
        f"-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY={out}",
    ]

    wrapper_dir = build / ("_dll_src" if args.dll else "_exe_src")
    generate_wrapper_cmakelists(root, wrapper_dir / "CMakeLists.txt", "dll" if args.dll else "exe")
    cmake_source = wrapper_dir

    rc = run_cmake(cmake_source, build, extra)
    if rc != 0:
        return rc

    settings = ensure_vscode_settings(root)
    if settings:
        print("[init_vscode] wrote:", settings)

    cc = build / "compile_commands.json"
    if not cc.exists():
        print("[init_vscode] WARNING: compile_commands.json not found.", file=sys.stderr)

    subprocess.run(["cmake", "--build", str(build), "--target", "ywlib_umbrellas"], check=False)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
