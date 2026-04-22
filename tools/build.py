from __future__ import annotations

import subprocess
import sys

from _common import project_root, run, run_python


def main() -> int:
    root = project_root()

    build_dir = root / "build"
    compile_commands_path = build_dir / "compile_commands.json"

    if not compile_commands_path.is_file():
        run_python(root / "tools" / "init.py", cwd=root)

    run(
        [
            "cmake",
            "--build",
            str(build_dir),
        ],
        cwd=root,
    )

    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as e:
        raise SystemExit(e.returncode)
    except Exception as e:
        print(f"error: {e}", file=sys.stderr)
        raise SystemExit(1)
