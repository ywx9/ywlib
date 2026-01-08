# ywlib template

This repository is a **template project** that provides a minimal C++ build environment
and the core library **ywlib**.

- `ywlib/yw` : platform-independent core
- `ywlib/ywx`: Windows / DirectX specific code
- Users are expected to freely add their own source files on top of this template.

This repository intentionally tracks **only the core library and tools**.
User code (such as `main.cpp`) is generated locally and not committed.

---

## Requirements

- CMake 3.20+
- C++20 or later compiler
  - Windows: w64devkit (MinGW-w64)
  - Linux: GCC or Clang
- Python 3 (for initialization scripts)

---

## Initialization

After cloning the repository, run the initialization script:

```bash
python tools/init.py
```

This script will:

1. Configure the project with CMake

2. Generate `compile_commands.json`

3. Create `.vscode/settings.json`

4. Create `main.cpp` if it does not exist

5. Create `CMakeLists.txt` if it does not exist

6. Generate umbrella headers (`ywlib`, `ywxlib`) if needed

## Project Structure (simplified)

```
.
├─ CMakeLists.txt
├─ .clang-format
├─ tools/
│  ├─ init_vscode.py
│  └─ templates/
│     └─ main.cpp.in
├─ ywlib/
│  ├─ yw/     (core headers)
│  ├─ ywx/    (Windows / DirectX headers)
│  ├─ ywlib   (generated umbrella header)
│  └─ ywxlib  (generated umbrella header)
└─ main.cpp   (generated locally, ignored by git)
```

## Build

### Configure

```bash
cmake -S . -B build
```

If you want to explicitly select a compiler:

```bash
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=g++
```

### Compile

```bash
cmake --build build
```

The executable name is defined at the top of `CMakeLists.txt`.

## Notes

- `main.cpp` is intentionally ignored by git.

- Users are free to add any additional source files or directories.

- Generated files such as `compile_commands.json` and umbrella headers are recreated automatically.

- Formatting is controlled by `.clang-format`.

## License

This project is provided as a template.<br>
You may use, modify, and redistribute it freely.
