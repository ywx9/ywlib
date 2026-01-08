#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path

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

    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text("\n".join(lines), encoding="utf-8")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
