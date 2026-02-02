#!/usr/bin/env python3
from __future__ import annotations

import argparse
import os
import tempfile
import time
from pathlib import Path


def atomic_write_text(path: Path, text: str, encoding: str = "utf-8",
                      retries: int = 1000, delay: float = 0.02) -> None:
    """
    Write text to `path` robustly on Windows:
    - write to a temp file in the same directory
    - os.replace() to atomically swap
    - retry on PermissionError (file watcher / AV / clangd race)
    """
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)

    last_err: Exception | None = None
    for _ in range(retries):
        tmp_path: Path | None = None
        try:
            fd, tmp_name = tempfile.mkstemp(
                prefix=path.name + ".", suffix=".tmp", dir=str(path.parent)
            )
            tmp_path = Path(tmp_name)

            # Use newline="\n" for stable output across environments
            with os.fdopen(fd, "w", encoding=encoding, newline="\n") as f:
                f.write(text)
                f.flush()
                os.fsync(f.fileno())

            os.replace(tmp_path, path)  # atomic-ish on Windows (replaces existing)
            return

        except PermissionError as e:
            # Typical: file is briefly locked by watcher / AV / indexer
            last_err = e
            time.sleep(delay)

        finally:
            if tmp_path is not None and tmp_path.exists():
                try:
                    tmp_path.unlink()
                except OSError:
                    pass

    raise PermissionError(
        f"[gen_umbrella] PermissionError: failed to write '{path}' after {retries} retries"
    ) from last_err


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--in-dir", required=True, help="Input directory containing headers")
    ap.add_argument("--out", required=True, help="Output umbrella header file path")
    ap.add_argument("--prefix", required=True, help='Include prefix used in generated includes (e.g. "yw" or "ywx")')
    ap.add_argument("--ext", action="append", default=[".h", ".hpp"], help="Header extensions to include")
    ap.add_argument("--preamble", action="append", default=[], help="Extra lines inserted after #pragma once")
    ap.add_argument("--exclude", action="append", default=[], help="Exclude by filename (exact match), can repeat")
    args = ap.parse_args()

    in_dir = Path(args.in_dir)
    out = Path(args.out)

    if not in_dir.exists():
        raise SystemExit(f"[gen_umbrella] input dir not found: {in_dir}")

    headers: list[Path] = []
    for ext in args.ext:
        headers.extend(in_dir.glob(f"*{ext}"))

    # Exclude by exact filename
    excl = set(args.exclude)
    headers = [p for p in headers if p.name not in excl]

    # Stable order
    headers = sorted(headers, key=lambda p: p.name.lower())

    lines: list[str] = ["#pragma once", ""]
    if args.preamble:
        lines.extend(args.preamble)
        lines.append("")

    # Use quoted include so it resolves via includePath (we add ${workspace}/ywlib)
    for h in headers:
        lines.append(f'#include "{args.prefix}/{h.name}"')
    lines.append("")

    # Ensure trailing newline for nicer diffs
    atomic_write_text(out, "\n".join(lines) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
