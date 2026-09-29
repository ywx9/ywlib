from __future__ import annotations

import json
import re
import shutil
import sys
from pathlib import Path


ROOT_DIR = Path(__file__).resolve().parents[1]
PROJECT_CONFIG_PATH = ROOT_DIR / "project.json"
BUILD_DIR = ROOT_DIR / "build"
COMPILE_COMMANDS_PATH = BUILD_DIR / "compile_commands.json"
VSCODE_DIR = ROOT_DIR / ".vscode"
SETTINGS_PATH = VSCODE_DIR / "settings.json"
YWLIB_DIR = ROOT_DIR / "ywlib"
UMBRELLA_HEADER_PATH = YWLIB_DIR / "ywlib"
HEADER_SUFFIXES = {".h", ".hh", ".hpp", ".hxx"}
DEFAULT_PROJECT_CONFIG: dict[str, object] = {
    "project_name": "test",
    "sources": ["source.cpp"],
    "include_dirs": ["ywlib"],
    "target_type": "exe",
    "ywlib_layer": "base",
    "cflags": ["-s", "-O2", "-fno-ident", "-fcontracts", "-freflection", "-ffunction-sections", "-fdata-sections"],
    "ldflags": ["-Wl,--gc-sections"],
    "show_console": True,
}

YWLIB_LAYERS = {"core": 0, "base": 1, "apps": 2}

APPS_LDFLAGS = [
    "-luser32", "-lgdi32", "-lole32", "-luuid",
    "-ld3d11", "-ldxgi", "-ld2d1", "-ldwrite", "-lwindowscodecs", "-lxaudio2_9",
]


def strip_jsonc(text: str) -> str:
    text = re.sub(r"//.*", "", text)
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    text = re.sub(r",(\s*[}\]])", r"\1", text)
    return text


def as_list(value: object, name: str) -> list[str]:
    if value is None:
        return []
    if isinstance(value, str):
        return [value]
    if isinstance(value, list) and all(isinstance(item, str) for item in value):
        return value
    raise ValueError(f"{name} must be a string or a list of strings")


def ywlib_layer(value: object) -> tuple[str, int]:
    if not isinstance(value, str) or value not in YWLIB_LAYERS:
        choices = ", ".join(YWLIB_LAYERS)
        raise ValueError(f"ywlib_layer must be one of: {choices}")
    level = YWLIB_LAYERS[value]
    if level >= YWLIB_LAYERS["apps"] and sys.platform != "win32":
        raise ValueError("ywlib_layer 'apps' requires a Windows host")
    return value, level


def load_project_config() -> dict[str, object]:
    if not PROJECT_CONFIG_PATH.exists():
        PROJECT_CONFIG_PATH.write_text(
            json.dumps(DEFAULT_PROJECT_CONFIG, indent=2, ensure_ascii=False) + "\n",
            encoding="utf-8",
            newline="\n",
        )
        print(f"created {PROJECT_CONFIG_PATH.relative_to(ROOT_DIR)}")

    config = json.loads(PROJECT_CONFIG_PATH.read_text(encoding="utf-8"))
    if not isinstance(config, dict):
        raise ValueError("project.json must contain a JSON object")
    return config


def load_settings() -> dict[str, object]:
    if not SETTINGS_PATH.exists():
        return {}

    text = SETTINGS_PATH.read_text(encoding="utf-8")
    if not text.strip():
        return {}

    settings = json.loads(strip_jsonc(text))
    if not isinstance(settings, dict):
        raise ValueError(".vscode/settings.json must contain a JSON object")
    return settings


def workspace_path(path: str) -> str:
    return "${workspaceFolder}/" + Path(path).as_posix()


def output_name(project_name: str, target_type: str) -> str:
    if target_type == "exe" and sys.platform == "win32":
        return f"{project_name}.exe"
    return project_name


def build_command(config: dict[str, object]) -> list[str]:
    project_name = str(config.get("project_name", "app"))
    compiler = str(config.get("compiler", "g++"))
    cpp_standard = str(config.get("cpp_standard", "c++26"))
    target_type = str(config.get("target_type", "exe"))
    output_dir = str(config.get("output_dir", "build"))

    sources = as_list(config.get("sources", config.get("source_file")), "sources")
    include_dirs = as_list(config.get("include_dirs", ["ywlib"]), "include_dirs")
    defines = as_list(config.get("defines"), "defines")
    cflags = as_list(config.get("cflags"), "cflags")
    ldflags = as_list(config.get("ldflags"), "ldflags")
    layer, layer_level = ywlib_layer(config.get("ywlib_layer", "base"))

    if not sources:
        raise ValueError("no source files configured")
    if target_type != "exe":
        raise ValueError(f"unsupported target_type: {target_type}")

    output_path = Path(output_dir) / output_name(project_name, target_type)
    command = [compiler, f"-std={cpp_standard}"]
    command.extend(cflags)
    command.extend(f"-D{define}" for define in defines)
    command.append(f"-DYWLIB_LAYER={layer_level}")
    command.extend(f"-I{include_dir}" for include_dir in include_dirs)
    command.extend(sources)
    command.extend(["-o", str(output_path)])
    if layer == "apps":
        command.extend(APPS_LDFLAGS)
    command.extend(ldflags)
    return command


def write_settings(config: dict[str, object]) -> None:
    compiler = str(config.get("compiler", "g++"))
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        raise FileNotFoundError(f"{compiler} was not found in PATH")

    cpp_standard = str(config.get("cpp_standard", "c++26"))
    include_dirs = as_list(config.get("include_dirs", ["ywlib"]), "include_dirs")
    defines = as_list(config.get("defines"), "defines")
    cflags = as_list(config.get("cflags"), "cflags")
    _, layer_level = ywlib_layer(config.get("ywlib_layer", "base"))

    settings = load_settings()
    associations = settings.get("files.associations")
    if not isinstance(associations, dict):
        associations = {}

    associations["ywlib"] = "cpp"
    settings["files.associations"] = associations
    settings["files.eol"] = "\n"
    settings["cmake.configureOnOpen"] = False
    settings["C_Cpp.default.compilerPath"] = compiler_path
    settings["C_Cpp.default.cppStandard"] = cpp_standard
    settings["C_Cpp.default.includePath"] = [workspace_path(path) for path in include_dirs]
    settings["C_Cpp.default.defines"] = defines
    settings["C_Cpp.default.compilerArgs"] = [*cflags, f"-DYWLIB_LAYER={layer_level}"]
    settings["C_Cpp.default.compileCommands"] = workspace_path("build/compile_commands.json")

    VSCODE_DIR.mkdir(parents=True, exist_ok=True)
    SETTINGS_PATH.write_text(
        json.dumps(settings, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
        newline="\n",
    )


def write_compile_commands(config: dict[str, object]) -> None:
    sources = as_list(config.get("sources", config.get("source_file")), "sources")
    command = build_command(config)
    entries = [
        {
            "directory": ROOT_DIR.as_posix(),
            "arguments": command,
            "file": Path(source).as_posix(),
        }
        for source in sources
    ]

    BUILD_DIR.mkdir(parents=True, exist_ok=True)
    COMPILE_COMMANDS_PATH.write_text(
        json.dumps(entries, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
        newline="\n",
    )


def find_headers() -> list[Path]:
    headers = []
    for path in YWLIB_DIR.rglob("*"):
        if not path.is_file():
            continue
        if path == UMBRELLA_HEADER_PATH:
            continue
        if path.suffix.lower() in HEADER_SUFFIXES:
            headers.append(path)
    return sorted(headers, key=lambda path: path.relative_to(YWLIB_DIR).as_posix())


def write_umbrella_header() -> None:
    if not YWLIB_DIR.exists():
        raise FileNotFoundError(f"missing directory: {YWLIB_DIR}")

    lines = [
        "// Generated by tools/init.py. Do not edit manually.",
        "#pragma once",
        "",
        "#ifndef YWLIB_LAYER",
        "#define YWLIB_LAYER 1",
        "#endif",
        "",
    ]
    headers_by_layer = {"core": [], "base": [], "apps": []}
    for header in find_headers():
        relative_path = header.relative_to(YWLIB_DIR)
        headers_by_layer[relative_path.parts[0]].append(relative_path.as_posix())

    for include_path in headers_by_layer["core"]:
        lines.append(f'#include "{include_path}"')
    lines.extend(["", "#if YWLIB_LAYER >= 1"])
    for include_path in headers_by_layer["base"]:
        lines.append(f'#include "{include_path}"')
    lines.extend(["#endif", "", "#if YWLIB_LAYER >= 2"])
    for include_path in headers_by_layer["apps"]:
        lines.append(f'#include "{include_path}"')
    lines.append("#endif")

    UMBRELLA_HEADER_PATH.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")


def main() -> int:
    try:
        config = load_project_config()
        write_settings(config)
        write_compile_commands(config)
        write_umbrella_header()
        print(f"updated {SETTINGS_PATH.relative_to(ROOT_DIR)}")
        print(f"updated {COMPILE_COMMANDS_PATH.relative_to(ROOT_DIR)}")
        print(f"updated {UMBRELLA_HEADER_PATH.relative_to(ROOT_DIR)}")
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"init.py: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
