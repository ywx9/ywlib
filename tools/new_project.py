#!/usr/bin/env python3
from __future__ import annotations

import os
import shutil
import stat
import subprocess
import sys
import tempfile
from pathlib import Path


TEMPLATE_REPO_URL = "https://github.com/ywx9/ywlib.git"


def run(cmd: list[str], cwd: Path | None = None) -> None:
    result = subprocess.run(
        cmd,
        cwd=str(cwd) if cwd else None,
        text=True,
    )
    if result.returncode != 0:
        raise SystemExit(result.returncode)


def remove_git_dir(path: Path) -> None:
    git_dir = path / ".git"

    def handle_remove_readonly(func, path, exc):
        os.chmod(path, stat.S_IWRITE)
        func(path)

    if git_dir.exists():
        shutil.rmtree(git_dir, onerror=handle_remove_readonly)


def copy_template(src: Path, dst: Path) -> None:
    if dst.exists():
        raise SystemExit(f"error: destination already exists: {dst}")
    shutil.copytree(src, dst)


def main() -> None:
    if len(sys.argv) != 2:
        print(f"usage: {Path(sys.argv[0]).name} PROJECT_NAME")
        raise SystemExit(2)

    project_name = sys.argv[1].strip()
    if not project_name:
        raise SystemExit("error: empty project name")

    destination = Path.cwd() / project_name

    with tempfile.TemporaryDirectory() as tmp:
        tmp_path = Path(tmp)
        clone_dir = tmp_path / "template"

        run(["git", "clone", "--depth", "1", TEMPLATE_REPO_URL, str(clone_dir)])
        remove_git_dir(clone_dir)
        copy_template(clone_dir, destination)

    print(f"created: {destination}")

    run(["python", f"{destination}/tools/init.py"])

    with open(f"{destination}/CMakeLists.txt", "r+", encoding="utf-8") as f:
        text = f.read()
        text = text.replace("__PROJECT_NAME__", project_name)
        f.seek(0)
        f.write(text)
        f.truncate()

if __name__ == "__main__":
    main()
