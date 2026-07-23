# AGENTS.MD

## Project

This is a C++23 cross-platform library project.
Windows / MinGW / CMake support is important.

## Build

1. Edit umbrella header instead of running `python tools/init.py`, if the list of headers changed.
2. Run `python tools/build.py` with approval.

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
- Keep reports concise: when the cause is identified, state only the conclusion and the minimal concrete fix.
