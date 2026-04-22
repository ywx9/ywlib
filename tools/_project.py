from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class ProjectConfig:
    project_name: str
    source_file: str
    target_type: str
    library: str

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

    if not project_name:
        raise ValueError("project_name is empty.")
    if not source_file:
        raise ValueError("source_file is empty.")
    if target_type not in {"exe", "dll"}:
        raise ValueError(f"target_type must be 'exe' or 'dll', got: {target_type!r}")
    if library not in {"ywlib", "ywxlib"}:
        raise ValueError(f"library must be 'ywlib' or 'ywxlib', got: {library!r}")

    return ProjectConfig(
        project_name=project_name,
        source_file=source_file,
        target_type=target_type,
        library=library,
    )
