from __future__ import annotations

from pathlib import Path


def _header_paths(include_dir: Path) -> list[Path]:
    if not include_dir.is_dir():
        return []
    return sorted(
        p for p in include_dir.rglob("*.h")
        if p.is_file()
    )


def _make_include_lines(include_dir: Path, headers: list[Path]) -> list[str]:
    lines: list[str] = []
    prefix = include_dir.name
    for path in headers:
        rel = path.relative_to(include_dir).as_posix()
        lines.append(f"#include <{prefix}/{rel}>")
    return lines


def generate_umbrella(include_dir: Path, output_path: Path, *, include_self: str | None = None) -> None:
    headers = _header_paths(include_dir)
    lines: list[str] = ["#pragma once", ""]

    if include_self:
        lines.append(f"#include <{include_self}>")
        lines.append("")

    lines.extend(_make_include_lines(include_dir, headers))
    lines.append("")

    output_path.write_text("\n".join(lines), encoding="utf-8", newline="\n")


def generate_all(ywlib_dir: Path) -> None:
    yw_dir = ywlib_dir / "yw"
    ywx_dir = ywlib_dir / "ywx"

    generate_umbrella(yw_dir, ywlib_dir / "ywlib")
    generate_umbrella(ywx_dir, ywlib_dir / "ywxlib", include_self="ywlib")
