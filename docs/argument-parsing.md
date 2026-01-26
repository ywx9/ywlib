# Argument Parsing in ywlib

## Overview

ywlib provides a lightweight command-line argument parsing module through `yw::args`. The parser automatically handles UTF-8 encoding on Windows and follows POSIX-style conventions for option parsing.

## Key Features

- **Unicode Support**: On Windows, arguments are obtained from `GetCommandLineW` and converted to UTF-8, bypassing the C runtime's potentially lossy argc/argv
- **POSIX-style Parsing**: Supports both short and long options with various value formats
- **Double Dash Separator**: Recognizes `--` as a separator between options and positional arguments
- **Negative Number Support**: Correctly handles negative numbers as option values (e.g., `--value -42`)
- **Multiple Values**: Options can appear multiple times with different values

## Core Components

### Global State

The `yw::args` namespace provides three global containers that hold parsed arguments:

```cpp
namespace yw::args {
  std::string program_name;              // Executable name (without extension)
  std::vector<option> options;           // Named options with their values
  std::vector<std::string> positionals;  // Positional arguments
}
```

### Option Structure

Each parsed option is represented by:

```cpp
struct option {
  std::string name;                    // Option name (with leading dashes)
  std::vector<std::string> values;     // All values for this option
};
```

## Usage

### Basic Parsing

Parse command-line arguments in your `main()` function:

```cpp
#include <ywlib>

int main(int argc, char** argv) {
  // Parse arguments
  auto result = yw::args::parse(argc, argv);
  if (!result) {
    yw::print("Failed to parse arguments: {}\n", result.error());
    return 1;
  }

  // Use parsed arguments
  yw::print("Program: {}\n", yw::args::program_name);

  return 0;
}
```

On Windows, the function automatically uses Windows APIs to get Unicode arguments:

```cpp
// On Windows, this is done automatically:
// - GetCommandLineW() retrieves the full command line
// - CommandLineToArgvW() splits it into arguments
// - Each argument is converted from UTF-16 to UTF-8
```

### Checking for Options

Use `has()` to check if an option was provided:

```cpp
if (yw::args::has("--verbose")) {
  // Enable verbose mode
}

if (yw::args::has("-h") || yw::args::has("--help")) {
  print_help();
  return 0;
}
```

### Retrieving Option Values

Get the first value of an option with `value()`:

```cpp
// Returns std::optional<std::string_view>
if (auto output = yw::args::value("--output")) {
  yw::print("Output file: {}\n", *output);
}

// With default value
auto level = yw::args::value("--level").value_or("info");
```

Get all values for an option that appears multiple times with `values()`:

```cpp
// Returns std::vector<std::string_view>
auto files = yw::args::values("--input");
for (auto file : files) {
  process_file(file);
}
```

### Accessing Positional Arguments

All non-option arguments are stored in `positionals`:

```cpp
if (yw::args::positionals.empty()) {
  yw::print("Error: no input files specified\n");
  return 1;
}

for (const auto& file : yw::args::positionals) {
  process_file(file);
}
```

## Parsing Rules

### Option Formats

The parser recognizes several option formats:

| Format | Example | Description |
|--------|---------|-------------|
| `--name value` | `--output file.txt` | Long option with separate value |
| `--name=value` | `--output=file.txt` | Long option with equals sign |
| `-n value` | `-o file.txt` | Short option with separate value |
| `-n=value` | `-o=file.txt` | Short option with equals sign |
| `--flag` | `--verbose` | Option without a value (empty string stored) |

### Value Assignment Rules

1. **Explicit Value (equals sign)**: `--option=value` always assigns `value` to `option`

2. **Next Token as Value**: If the next token is not an option, it becomes the value:
   ```bash
   --output file.txt  # "file.txt" is the value
   ```

3. **Negative Numbers**: Numbers starting with `-` followed by a digit are treated as values:
   ```bash
   --threshold -42    # "-42" is the value, not an option
   ```

4. **No Value Available**: If the option is the last token or the next token is another option:
   ```bash
   --verbose          # Empty string value
   --output --verbose # "--output" gets empty string, "--verbose" is another option
   ```

### Special Tokens

- **`-`** (single dash): Not treated as an option; becomes a positional argument (commonly means stdin/stdout)
- **`--`** (double dash): Separator; all following tokens are positional arguments, even if they start with `-`

### Example Command Lines

```bash
# Mixed options and positionals
program --verbose -o output.txt input1.txt input2.txt

# Parsing result:
# options: [{"--verbose", {""}}, {"-o", {"output.txt"}}]
# positionals: ["input1.txt", "input2.txt"]

# Using equals sign
program --output=result.txt --level=3 file.txt

# Parsing result:
# options: [{"--output", {"result.txt"}}, {"--level", {"3"}}]
# positionals: ["file.txt"]

# Multiple values for same option
program --input a.txt --input b.txt --input c.txt

# Parsing result:
# options: [{"--input", {"a.txt", "b.txt", "c.txt"}}]
# positionals: []

# Negative numbers
program --threshold -42 --offset -3.14

# Parsing result:
# options: [{"--threshold", {"-42"}}, {"--offset", {"-3.14"}}]

# Double dash separator
program --verbose -- --not-an-option -file.txt

# Parsing result:
# options: [{"--verbose", {""}}]
# positionals: ["--not-an-option", "-file.txt"]

# Single dash (stdin/stdout convention)
program --input - --output -

# Parsing result:
# options: [{"--input", {"-"}}, {"--output", {"-"}}]
# positionals: []
```

## Complete Example

```cpp
#include <ywlib>

void print_usage() {
  yw::print(R"(Usage: {} [OPTIONS] FILES...

Options:
  -h, --help           Show this help message
  -v, --verbose        Enable verbose output
  -o, --output FILE    Output file (default: stdout)
  -f, --format FMT     Output format (json|xml|text)
  -I, --include DIR    Add include directory (can be repeated)
  --                   Treat remaining arguments as files

Examples:
  {} -v --output result.txt input.txt
  {} -I /usr/include -I /opt/include main.cpp
  {} -- --file-with-dashes.txt
)", yw::args::program_name);
}

int main(int argc, char** argv) {
  // Parse arguments
  if (auto result = yw::args::parse(argc, argv); !result) {
    yw::print(stderr, "Error: {}\n", result.error());
    return 1;
  }

  // Handle help
  if (yw::args::has("-h") || yw::args::has("--help")) {
    print_usage();
    return 0;
  }

  // Get options
  bool verbose = yw::args::has("-v") || yw::args::has("--verbose");
  auto output_file = yw::args::value("-o").or_else([] {
    return yw::args::value("--output");
  });
  auto format = yw::args::value("--format").value_or("text");

  // Get multiple values
  auto include_dirs = yw::args::values("-I");
  auto include_dirs2 = yw::args::values("--include");

  // Combine short and long forms
  std::vector<std::string_view> all_includes;
  all_includes.insert(all_includes.end(), include_dirs.begin(), include_dirs.end());
  all_includes.insert(all_includes.end(), include_dirs2.begin(), include_dirs2.end());

  // Check positional arguments
  if (yw::args::positionals.empty()) {
    yw::print(stderr, "Error: no input files specified\n");
    print_usage();
    return 1;
  }

  // Process files
  if (verbose) {
    yw::print("Processing {} files...\n", yw::args::positionals.size());
    if (!all_includes.empty()) {
      yw::print("Include directories:\n");
      for (auto dir : all_includes) {
        yw::print("  - {}\n", dir);
      }
    }
  }

  for (const auto& file : yw::args::positionals) {
    if (verbose) {
      yw::print("Processing: {}\n", file);
    }
    // Process file...
  }

  if (output_file) {
    yw::print("Output written to: {}\n", *output_file);
  }

  return 0;
}
```

## Implementation Notes

### Platform-Specific Behavior

#### Windows

On Windows (`_WIN32` or `_WIN64` defined):
- Uses `GetCommandLineW()` to get the full Unicode command line
- Uses `CommandLineToArgvW()` to split into arguments
- Converts each argument from UTF-16 to UTF-8
- This ensures proper handling of non-ASCII characters in file paths and arguments

#### Unix/Linux

On other platforms:
- Uses the standard `argc`/`argv` from main()
- Assumes arguments are already in the correct encoding (typically UTF-8)

### Error Handling

The `parse()` function returns `std::expected<void, error_trace>`:

```cpp
auto result = yw::args::parse(argc, argv);
if (!result) {
  // Error occurred (typically only on Windows if CommandLineToArgvW fails)
  yw::print(stderr, "{}\n", result.error());
  return 1;
}
```

On Windows, parsing can fail if:
- `CommandLineToArgvW()` fails (extremely rare, indicates system issues)

On other platforms, parsing never fails.

### Performance Considerations

- Argument parsing happens once at program startup
- Results are stored in global variables for O(1) access
- Option lookup is linear search (O(n) where n = number of unique options)
- For most command-line programs, this is negligible

## Best Practices

### 1. Parse Early

Parse arguments at the very beginning of `main()`:

```cpp
int main(int argc, char** argv) {
  if (auto r = yw::args::parse(argc, argv); !r) {
    yw::print(stderr, "Parse error: {}\n", r.error());
    return 1;
  }

  // Rest of program...
}
```

### 2. Support Both Short and Long Forms

Users appreciate having both options:

```cpp
if (yw::args::has("-v") || yw::args::has("--verbose")) {
  // ...
}

auto output = yw::args::value("-o").or_else([] {
  return yw::args::value("--output");
});
```

### 3. Provide Defaults

Use `.value_or()` for sensible defaults:

```cpp
auto log_level = yw::args::value("--log-level").value_or("info");
auto max_threads = yw::args::value("--threads").value_or("4");
```

### 4. Validate Input

Always validate option values:

```cpp
auto level_str = yw::args::value("--level").value_or("1");
int level;
auto [ptr, ec] = std::from_chars(level_str.data(),
                                  level_str.data() + level_str.size(),
                                  level);
if (ec != std::errc{}) {
  yw::print(stderr, "Invalid level: {}\n", level_str);
  return 1;
}
if (level < 1 || level > 10) {
  yw::print(stderr, "Level must be between 1 and 10\n");
  return 1;
}
```

### 5. Document Your Interface

Provide a `--help` option with clear documentation:

```cpp
if (yw::args::has("--help") || yw::args::has("-h")) {
  print_help();
  return 0;
}
```

## Comparison with Other Libraries

| Feature | ywlib | getopt | CLI11 | argparse |
|---------|-------|--------|-------|----------|
| Header-only | ✓ | - | ✓ | ✓ |
| C++ version | C++23 | C | C++11 | C++17 |
| Unicode on Windows | ✓ | - | ✓ | - |
| Type safety | Manual | - | ✓ | ✓ |
| Subcommands | - | - | ✓ | ✓ |
| Validation | Manual | - | ✓ | ✓ |
| Size | Tiny | Small | Large | Medium |
| Dependencies | None | None | None | None |

**ywlib's philosophy**: Provide a minimal, correct foundation that's easy to extend for specific needs rather than a heavy framework.

## See Also

- [yw/args.h](../ywlib/yw/args.h) - Source code with implementation
- [yw/core.h](../ywlib/yw/core.h) - Core utilities and `std::expected` error handling
