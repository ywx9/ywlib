from __future__ import annotations

import argparse
import os
import shutil
import stat
import subprocess
from pathlib import Path


TEMPLATE_REPOSITORY_URL = "https://github.com/ywx9/ywlib.git"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Clone a project template, detach it from the original git history, and create project.json."
    )
    parser.add_argument(
        "project_name",
        help="Directory name of the new project.",
    )
    parser.add_argument(
        "--repo",
        default=TEMPLATE_REPOSITORY_URL,
        help="Template repository URL.",
    )
    parser.add_argument(
        "--branch",
        default="main",
        help="Branch to clone.",
    )
    parser.add_argument(
        "--output-dir",
        default=".",
        help="Parent directory where the project will be created.",
    )
    return parser.parse_args()


def run(args: list[str], *, cwd: Path | None = None) -> None:
    print("+", " ".join(args))
    subprocess.run(
        args,
        cwd=str(cwd) if cwd is not None else None,
        check=True,
    )


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8", newline="\n")


def render_template(template_text: str, values: dict[str, str]) -> str:
    result = template_text
    for key, value in values.items():
        result = result.replace(f"@{key}@", value)
    return result


def _handle_remove_readonly(func, path, exc_info) -> None:
    ex = exc_info[1]
    if not isinstance(ex, PermissionError):
        raise ex

    os.chmod(path, stat.S_IWRITE)
    func(path)


def remove_git_dir(project_dir: Path) -> None:
    git_dir = project_dir / ".git"
    if git_dir.exists():
        shutil.rmtree(git_dir, onerror=_handle_remove_readonly)


def create_project_json(project_dir: Path, project_name: str) -> None:
    template_path = project_dir / "tools" / "templates" / "project.json.in"
    output_path = project_dir / "project.json"

    if output_path.exists():
        return

    text = render_template(
        read_text(template_path),
        {
            "PROJECT_NAME": project_name,
        },
    )
    write_text(output_path, text)


def main() -> int:
    args = parse_args()

    output_dir = Path(args.output_dir).resolve()
    project_dir = output_dir / args.project_name

    if project_dir.exists():
        raise FileExistsError(f"Destination already exists: {project_dir}")

    output_dir.mkdir(parents=True, exist_ok=True)

    run(
        [
            "git",
            "clone",
            "--branch",
            args.branch,
            "--single-branch",
            args.repo,
            str(project_dir),
        ]
    )

    remove_git_dir(project_dir)
    create_project_json(project_dir, args.project_name)

    print(f"Project created: {project_dir}")
    print("Next steps:")
    print("  1. Edit project.json")
    print("  2. Run: python tools/init.py")
    print("  3. Edit source file")
    print("  4. Run: python tools/build.py")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
