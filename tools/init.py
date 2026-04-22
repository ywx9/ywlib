from __future__ import annotations

import shutil
import subprocess
import sys

from _common import cmake_configure, ensure_file_from_template, project_root, templates_dir
from _gen_umbrella import generate_all
from _project import load_project_config


def main() -> int:
    root = project_root()
    template_dir = templates_dir()

    project_json_path = root / "project.json"
    if not project_json_path.is_file():
        raise FileNotFoundError(f"project.json not found: {project_json_path}")

    config = load_project_config(project_json_path)

    values = {
        "PROJECT_NAME": config.project_name,
        "SOURCE_FILE": config.source_file,
        "TARGET_TYPE": config.target_type,
        "LIBRARY": config.library,
    }

    generate_all(root / "ywlib")

    ensure_file_from_template(
        root / "CMakeLists.txt",
        template_dir / "CMakeLists.txt.in",
        values,
        overwrite=True,
    )

    ensure_file_from_template(
        root / ".vscode" / "settings.json",
        template_dir / "settings.json.in",
        values,
        overwrite=True,
    )

    source_template_name = (
        "source_ywxlib.cpp.in" if config.library == "ywxlib" else "source_ywlib.cpp.in"
    )
    ensure_file_from_template(
        root / config.source_file,
        template_dir / source_template_name,
        values,
        overwrite=False,
    )

    build_dir = root / "build"
    if build_dir.exists():
        shutil.rmtree(build_dir)

    cmake_configure(build_dir)

    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as e:
        raise SystemExit(e.returncode)
    except Exception as e:
        print(f"error: {e}", file=sys.stderr)
        raise SystemExit(1)
