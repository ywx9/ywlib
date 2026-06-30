# AGENTS.MD

## Project
This is a C++23 cross-platform library project.
Windows / MinGW / CMake support is important.

## Build
Run:
`python tools/build.py`

If the number of headers changed, run first:
`python tools/init.py`

For DLL:
python tools/build.py --dll

## Style
- Public APIs should avoid exceptions where possible.
- Prefer std::expected<T, error_trace>.
- Keep headers self-contained.
- Do not introduce external Python dependencies.
- Preserve existing namespace and file layout.
- Do not change public APIs unless explicitly requested.

## Notes
- Read source files and comments as UTF-8 by default. Do not rely on PowerShell's default text encoding when inspecting files.
