from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Mapping


@dataclass(frozen=True)
class ProjectConfig:
    project_name: str
    source_file: str
    target_type: str
    library: str
    show_console: bool

    @property
    def source_path(self) -> Path:
        return Path(self.source_file)


def load_project_config(project_json_path: Path) -> ProjectConfig:
    with project_json_path.open("r", encoding="utf-8") as f:
        data = json.load(f)

    project_name = str(data.get("project_name", "")).strip()
    source_file = str(data.get("source_file", "")).strip()
    target_type = str(data.get("target_type", "")).strip()
    library = str(data.get("library", "")).strip()
    show_console = data.get("show_console", True)

    if not project_name:
        raise ValueError("project_name is empty.")
    if not source_file:
        raise ValueError("source_file is empty.")
    if target_type not in {"exe", "dll"}:
        raise ValueError(f"target_type must be 'exe' or 'dll', got: {target_type!r}")
    if library not in {"ywlib", "ywxlib"}:
        raise ValueError(f"library must be 'ywlib' or 'ywxlib', got: {library!r}")
    if not isinstance(show_console, bool):
        raise ValueError(
            f"show_console must be true or false, got: {show_console!r}"
        )

    return ProjectConfig(
        project_name=project_name,
        source_file=source_file,
        target_type=target_type,
        library=library,
        show_console=show_console,
    )


def project_template_values(config: ProjectConfig) -> Mapping[str, str]:
    return {
        "PROJECT_NAME": config.project_name,
        "SOURCE_FILE": config.source_file,
        "TARGET_TYPE": config.target_type,
        "LIBRARY": config.library,
        "EXECUTABLE_OPTIONS": "" if config.show_console else "WIN32",
    }
