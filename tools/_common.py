from __future__ import annotations

import subprocess
import sys
from pathlib import Path
from typing import Mapping


def project_root() -> Path:
    return Path(__file__).resolve().parent.parent


def tools_dir() -> Path:
    return project_root() / "tools"


def templates_dir() -> Path:
    return tools_dir() / "templates"


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8", newline="\n")


def render_template(template_text: str, values: Mapping[str, str]) -> str:
    result = template_text
    for key, value in values.items():
        result = result.replace(f"@{key}@", value)
    return result


def render_template_file(template_path: Path, values: Mapping[str, str]) -> str:
    return render_template(read_text(template_path), values)


def ensure_file_from_template(
    output_path: Path,
    template_path: Path,
    values: Mapping[str, str],
    *,
    overwrite: bool = False,
) -> bool:
    if output_path.exists() and not overwrite:
        return False
    text = render_template_file(template_path, values)
    write_text(output_path, text)
    return True


def run(
    args: list[str],
    *,
    cwd: Path | None = None,
    check: bool = True,
) -> subprocess.CompletedProcess[str]:
    print("+", " ".join(args))
    return subprocess.run(
        args,
        cwd=str(cwd) if cwd is not None else None,
        check=check,
        text=True,
    )


def run_python(
    script_path: Path,
    script_args: list[str] | None = None,
    *,
    cwd: Path | None = None,
    check: bool = True,
) -> subprocess.CompletedProcess[str]:
    args = [sys.executable, str(script_path)]
    if script_args:
        args.extend(script_args)
    return run(args, cwd=cwd, check=check)


def cmake_configure(
    build_dir: Path,
    *,
    generator: str = "MinGW Makefiles",
) -> None:
    root = project_root()
    build_dir.mkdir(parents=True, exist_ok=True)
    run(
        [
            "cmake",
            "-S",
            str(root),
            "-B",
            str(build_dir),
            "-G",
            generator,
            "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
        ]
    )
