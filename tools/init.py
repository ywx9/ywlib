#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import platform
import re
import shutil
import subprocess
import sys
from pathlib import Path

# ------------------------------------------------------------
# helpers
# ------------------------------------------------------------

def ensure_main_cpp(workspace: Path):
    main_cpp = workspace / "main.cpp"
    if main_cpp.exists(): return
    template = workspace / "tools" / "templates" / "main.cpp.in"
    if not template.exists(): raise RuntimeError("main.cpp template not found")
    print("[init_vscode] creating main.cpp from template")
    main_cpp.write_text(template.read_text(encoding="utf-8"), encoding="utf-8")

def ensure_vscode_settings(workspace: Path):
    settings = workspace / ".vscode" / "settings.json"
    if settings.exists(): return None
    template = workspace / "tools" / "templates" / "settings.json.in"
    if not template.exists(): raise RuntimeError("settings.json template not found")
    print("[init_vscode] creating .vscode/settings.json from template")
    settings.write_text(template.read_text(encoding="utf-8"), encoding="utf-8")
    return settings

def which_or_error(name: str) -> str:
    p = shutil.which(name)
    if not p: raise RuntimeError(f"{name} not found in PATH")
    return p

def pick_mingw_generator() -> str:
    # Prefer Ninja if available
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

def generate_dll_cmakelists(base_cmake: Path, generated_cmake: Path, project_root: Path) -> None:
    src_text = base_cmake.read_text(encoding="utf-8")

    exe_block_pattern = re.compile(
        r"if \(WIN32 AND !SHOW_CONSOLE\)\s*"
        r"add_executable\(\$\{YOUR_APP_NAME\} WIN32 main\.cpp\)\s*"
        r"else\(\)\s*"
        r"add_executable\(\$\{YOUR_APP_NAME\} main\.cpp\)\s*"
        r"endif\(\)",
        flags=re.MULTILINE,
    )

    dll_target = (
        'add_library(${YOUR_APP_NAME} SHARED "${YWLIB_PROJECT_ROOT}/main.cpp")\n\n'
        '# DLL output naming (user-configurable)\n'
        'set(YOUR_DLL_PREFIX "" CACHE STRING "Prefix for DLL/shared library filename")\n'
        'set(YOUR_DLL_OUTPUT_NAME "${YOUR_APP_NAME}" CACHE STRING "Output filename base for DLL/shared library")\n'
        'set_target_properties(${YOUR_APP_NAME} PROPERTIES\n'
        '  PREFIX "${YOUR_DLL_PREFIX}"\n'
        '  OUTPUT_NAME "${YOUR_DLL_OUTPUT_NAME}"\n'
        ')'
    )

    if exe_block_pattern.search(src_text):
        cmake_text = exe_block_pattern.sub(dll_target, src_text, count=1)
    else:
        cmake_text = src_text.replace("add_executable(${YOUR_APP_NAME} WIN32 main.cpp)", dll_target)
        cmake_text = cmake_text.replace("add_executable(${YOUR_APP_NAME} main.cpp)", dll_target)

    cmake_text = cmake_text.replace("${CMAKE_CURRENT_SOURCE_DIR}", "${YWLIB_PROJECT_ROOT}")
    cmake_text = f'set(YWLIB_PROJECT_ROOT "{project_root.resolve().as_posix()}")\n\n' + cmake_text

    generated_cmake.parent.mkdir(parents=True, exist_ok=True)
    generated_cmake.write_text(cmake_text, encoding="utf-8")
    print("[init_vscode] generated dll cmakelists:", generated_cmake)

def ensure_cmakelists(workspace: Path):
    cmakelists = workspace / "CMakeLists.txt"
    if cmakelists.exists():
        return
    template = workspace / "tools" / "templates" / "CMakeLists.txt.in"
    if not template.exists():
        raise RuntimeError("CMakeLists.txt template not found")
    print("[init_vscode] creating CMakeLists.txt from template")
    cmakelists.write_text(template.read_text(encoding="utf-8"), encoding="utf-8")

# ------------------------------------------------------------
# main
# ------------------------------------------------------------

def main() -> int:
    workspace = Path(__file__).resolve().parents[1]
    ensure_main_cpp(workspace)
    ensure_cmakelists(workspace)

    ap = argparse.ArgumentParser()
    ap.add_argument(
        "--toolchain",
        choices=["gcc", "clang", "msvc", "auto"],
        default="gcc",
        help="Toolchain to use (default: gcc)",
    )
    ap.add_argument("--dll", action="store_true", help="Configure build as DLL (without modifying root CMakeLists.txt)")
    ap.add_argument("--build-dir", default="build")
    ap.add_argument("--output-dir", default=None, help="Directory for final binaries (.exe/.dll). Default: same as --build-dir")
    ap.add_argument("--reset-build", action="store_true")
    ap.add_argument("--reset-vscode", action="store_true")
    args = ap.parse_args()

    root = workspace
    build = root / args.build_dir
    output_dir = (root / args.output_dir) if args.output_dir else build
    vscode = root / ".vscode"
    settings = vscode / "settings.json"

    if args.reset_build and build.exists():
        shutil.rmtree(build)
    if args.reset_vscode and vscode.exists():
        shutil.rmtree(vscode)

    build.mkdir(exist_ok=True)
    output_dir.mkdir(parents=True, exist_ok=True)

    extra: list[str] = []

    try:
        if args.toolchain == "gcc":
            gen = pick_mingw_generator()
            gcc = which_or_error("gcc")
            gpp = which_or_error("g++")
            extra += [
                "-G", gen,
                f"-DCMAKE_CXX_COMPILER={gpp}",
            ]

        elif args.toolchain == "clang":
            clang = which_or_error("clang")
            clangpp = which_or_error("clang++")
            # generatorは指定しない（Ninjaが入っていればCMakeが選びやすい）
            extra += [
                f"-DCMAKE_CXX_COMPILER={clangpp}",
            ]

        elif args.toolchain == "msvc":
            if platform.system().lower() != "windows":
                raise RuntimeError("msvc toolchain is only valid on Windows")
            # 明示的にVS generatorを指定
            extra += ["-G", "Visual Studio 17 2022"]

        elif args.toolchain == "auto":
            # 完全にCMakeに任せる
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

    cmake_source = root
    if args.dll:
        generated_source = build / "_dll_src"
        generate_dll_cmakelists(root / "CMakeLists.txt", generated_source / "CMakeLists.txt", root)
        cmake_source = generated_source

    rc = run_cmake(cmake_source, build, extra)
    if rc != 0:
        return rc

    # --------------------------------------------------------
    # VS Code settings (minimal)
    # --------------------------------------------------------
    settings = ensure_vscode_settings(root)
    if settings: print("[init_vscode] wrote:", settings)

    cc = build / "compile_commands.json"
    if not cc.exists():
        print(
            "[init_vscode] WARNING: compile_commands.json not found.",
            file=sys.stderr,
        )

    subprocess.run(["cmake", "--build", str(build), "--target", "ywlib_umbrellas"], check=False)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
