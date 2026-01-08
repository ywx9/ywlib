#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import platform
import shutil
import subprocess
import sys
from pathlib import Path

# ------------------------------------------------------------
# helpers
# ------------------------------------------------------------

def ensure_main_cpp(workspace: Path):
    main_cpp = workspace / "main.cpp"
    if main_cpp.exists():
        return

    template = workspace / "tools" / "templates" / "main.cpp.in"
    if not template.exists():
        raise RuntimeError("main.cpp template not found")

    print("[init_vscode] creating main.cpp from template")
    main_cpp.write_text(template.read_text(encoding="utf-8"), encoding="utf-8")

def which_or_error(name: str) -> str:
    p = shutil.which(name)
    if not p:
        raise RuntimeError(f"{name} not found in PATH")
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

# ------------------------------------------------------------
# main
# ------------------------------------------------------------

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "--toolchain",
        choices=["gcc", "clang", "msvc", "auto"],
        default="gcc",
        help="Toolchain to use (default: gcc)",
    )
    ap.add_argument("--build-dir", default="build")
    ap.add_argument("--reset-build", action="store_true")
    ap.add_argument("--reset-vscode", action="store_true")
    args = ap.parse_args()

    root = Path.cwd()
    build = root / args.build_dir
    vscode = root / ".vscode"
    settings = vscode / "settings.json"

    if args.reset_build and build.exists():
        shutil.rmtree(build)
    if args.reset_vscode and vscode.exists():
        shutil.rmtree(vscode)

    build.mkdir(exist_ok=True)

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

    rc = run_cmake(root, build, extra)
    if rc != 0:
        return rc

    # --------------------------------------------------------
    # VS Code settings (minimal)
    # --------------------------------------------------------
    vscode.mkdir(exist_ok=True)

    content = {
        "files.associations": {
            "ywlib": "cpp",
            "ywxlib": "cpp",
        },
        "C_Cpp.default.compileCommands":
            f"${{workspaceFolder}}/{args.build_dir}/compile_commands.json",
    }

    settings.write_text(
        json.dumps(content, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )

    print("[init_vscode] wrote:", settings)

    cc = build / "compile_commands.json"
    if not cc.exists():
        print(
            "[init_vscode] WARNING: compile_commands.json not found.",
            file=sys.stderr,
        )

    subprocess.run(["cmake", "--build", str(build), "--target", "ywlib_umbrellas"], check=False)

    workspace = Path(__file__).resolve().parents[1]
    ensure_main_cpp(workspace)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
