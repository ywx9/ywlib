# AGENTS.MD

## Project

This is a C++23 cross-platform library project.
Windows / MinGW / CMake support is important.

## How to Test

1. AI may freely create and edit `build/test.cpp` for verification. This is the only
   permitted location for verification source files; do not create other test files
   or test directories such as `tools/tests/`.
2. Include the umbrella header (`<ywlib>` or `<ywxlib>`) and exercise the changed APIs.
3. Run `python tools/test.py` to compile and link, or add `--debug` for Debug settings.
   The script copies the root `CMakeLists.txt` into `build/`, adjusts the source and
   include paths, and builds in `build/test-build/`. It deletes the copied
   `CMakeLists.txt` on completion, including build failure; the original is unchanged.
4. This command does not execute the compiled program. Report compilation and runtime
   verification separately.

## Style

- Public APIs should avoid exceptions where possible.
- Prefer yw::result<T>.

## Notes

- Edit the umbrella header instead of running `python tools/init.py` when the header list changes.
- Do not introduce external Python dependencies.
- Read source files and comments as UTF-8 by default.
  Do not rely on PowerShell's default text encoding when inspecting files.
- Do not routinely run `git status` or broad `git diff`.
- Source file named in `project.json` is gitignored and located in the root directory.
  Do not rely on `rg --files` to find it.
- Do not include invisual library headers; Use umbrella header (`<ywlib>` or `<ywxlib>`).
