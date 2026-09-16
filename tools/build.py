from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path


ROOT_DIR = Path(__file__).resolve().parents[1]
CONFIG_PATH = ROOT_DIR / "project.json"


def as_list(value: object, name: str) -> list[str]:
    if value is None:
        return []
    if isinstance(value, str):
        return [value]
    if isinstance(value, list) and all(isinstance(item, str) for item in value):
        return value
    raise ValueError(f"{name} must be a string or a list of strings")


def load_config() -> dict[str, object]:
    if not CONFIG_PATH.exists():
        raise FileNotFoundError(f"missing config file: {CONFIG_PATH}")
    with CONFIG_PATH.open("r", encoding="utf-8") as file:
        config = json.load(file)
    if not isinstance(config, dict):
        raise ValueError("project.json must contain a JSON object")
    return config


def output_name(project_name: str, target_type: str) -> str:
    if target_type == "exe" and sys.platform == "win32":
        return f"{project_name}.exe"
    return project_name


def main() -> int:
    try:
        config = load_config()

        project_name = str(config.get("project_name", "app"))
        compiler = str(config.get("compiler", "g++"))
        cpp_standard = str(config.get("cpp_standard", "c++26"))
        target_type = str(config.get("target_type", "exe"))
        output_dir = ROOT_DIR / str(config.get("output_dir", "build"))

        sources = as_list(config.get("sources", config.get("source_file")), "sources")
        include_dirs = as_list(config.get("include_dirs", ["ywlib"]), "include_dirs")
        defines = as_list(config.get("defines"), "defines")
        cflags = as_list(config.get("cflags"), "cflags")
        ldflags = as_list(config.get("ldflags"), "ldflags")

        if not sources:
            raise ValueError("no source files configured")
        if target_type != "exe":
            raise ValueError(f"unsupported target_type: {target_type}")

        output_dir.mkdir(parents=True, exist_ok=True)
        output_path = output_dir / output_name(project_name, target_type)

        command = [compiler, f"-std={cpp_standard}"]
        command.extend(cflags)
        command.extend(f"-D{define}" for define in defines)
        command.extend(f"-I{include_dir}" for include_dir in include_dirs)
        command.extend(sources)
        command.extend(["-o", str(output_path)])
        command.extend(ldflags)

        print(" ".join(command))
        result = subprocess.run(command, cwd=ROOT_DIR)
        return result.returncode
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"build.py: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
