# AGENTS.MD

## Project

This is a C++23 cross-platform library project.
Windows / MinGW / CMake support is important.

## Build

Run:
`python tools/build.py`

If the list of headers changed:
`python tools/init.py`
`python tools/build.py`

For building DLL insted of EXE:
`python tools/build.py --dll`

Build commands require approval and must be run outside the Windows sandbox.
Do not create or redirect to a workspace-local temporary directory.

## Style

- Public APIs should avoid exceptions where possible.
- Prefer std::expected<T, error>.

## Notes

- Do not introduce external Python dependencies.
- Do not change public APIs unless explicitly requested.
- Read source files and comments as UTF-8 by default.
  Do not rely on PowerShell's default text encoding when inspecting files.
- Do not routinely run `git status` or broad `git diff`.
- Never assume the working tree is clean, and never revert or overwrite unrelated user changes.
