# Error Handling in ywlib

## Overview

ywlib uses C++23's `std::expected` for error handling, providing a modern, exception-free approach to managing errors. The error system is designed to capture detailed error information including error codes, messages, file positions, and stack traces of source locations.

## Core Components

### Error Codes

The library defines standard error codes through the `yw::errors` enum:

```cpp
enum class errors : uint32_t {
  success = 0,
  invalid_argument,
  invalid_file,
  invalid_operation,
  operation_failed,
  not_initialized,
};
```

### Error Structure

The `yw::error` struct contains comprehensive error information:

```cpp
struct error {
  errors code;              // Error code from the errors enum
  int system_code;          // System-specific error code (e.g., errno, GetLastError())
  uint64_t position;        // Position in input (useful for parsers), -1 if not applicable
  null_terminated<char> message;  // Human-readable error message
};
```

**Fields:**
- `code`: The primary error classification
- `system_code`: Operating system error code (0 if not applicable)
- `position`: Byte offset in input data for parsing errors (uint64_t(-1) if not applicable)
- `message`: Descriptive error message

### Error Trace

The `yw::error_trace` struct extends error with a stack trace:

```cpp
struct error_trace {
  yw::error error;
  std::vector<source> frames;

  error_trace& push(const source& src = {}) &;
};
```

This allows tracking error propagation through multiple function calls, similar to exception stack traces.

### Source Location

The `yw::source` struct captures where errors occur:

```cpp
struct source {
  std::string_view file;     // Source file name
  std::string_view func;     // Function name
  uint32_t line;             // Line number
  uint32_t column;           // Column number
};
```

Source locations are automatically captured using `std::source_location::current()`.

## Usage Patterns

### Returning Errors

Functions that may fail should return `std::expected<T, error_trace>`:

```cpp
std::expected<int, error_trace> parse_number(std::string_view str) {
  if (str.empty()) {
    return unexpected_error(
      errors::invalid_argument,
      "empty string cannot be parsed"
    );
  }
  // ... parsing logic ...
  return result;
}
```

### Creating Errors

Use the `unexpected_error` helper functions:

```cpp
// Basic error
auto err = unexpected_error(errors::invalid_argument, "invalid input");

// Error with system code
auto err = unexpected_error(errors::invalid_file, "cannot open file", errno);

// Error with position (for parsers)
auto err = unexpected_error(errors::invalid_argument, "unexpected character", 0, position);

// Error with source location (captured automatically)
auto err = unexpected_error(errors::operation_failed, "operation failed");
```

### Propagating Errors

When a function receives an error and wants to add its own frame to the trace:

```cpp
std::expected<Result, error_trace> high_level_function() {
  auto result = low_level_function();
  if (!result) {
    return unexpected_error(result.error());  // Adds current source location
  }
  return process(*result);
}
```

Or manually add a frame:

```cpp
if (!result) {
  result.error().push();  // Adds current source location to the trace
  return result;
}
```

### Checking Results

Standard `std::expected` idioms apply:

```cpp
auto result = risky_operation();

// Check if successful
if (result) use_value(*result);

// Check for error
if (!result) handle_error(result.error());

// Value-or pattern
auto value = risky_operation().value_or(default_value);

// Transform pattern
auto transformed = risky_operation()
  .transform([](auto v) { return process(v); });
```

### Error Handling Example

```cpp
std::expected<Document, error_trace> load_document(const std::filesystem::path& path) {
  // Try to open the file
  auto file_result = open_file(path);
  if (!file_result) {
    return unexpected_error(file_result.error());  // Propagate with new frame
  }

  // Try to read content
  auto content = read_content(*file_result);
  if (!content) {
    return unexpected_error(content.error());
  }

  // Try to parse
  auto doc = parse_document(*content);
  if (!doc) {
    return unexpected_error(
      errors::invalid_file,
      yw::format("Failed to parse document from {}", path)
    );
  }

  return *doc;
}
```

## Error Formatting

ywlib provides formatters for all error types, enabling easy error reporting:

```cpp
auto result = parse_xml(xml_string);
if (!result) {
  yw::print("Error: {}", result.error());
  // Output includes:
  // - Error message
  // - System code (if applicable)
  // - Input position (if applicable)
  // - Stack trace of source locations
}
```

**Example output:**
```
Failed to parse XML: unexpected character '<' (code=42)
  input offset=145
  at parse_element() in xml.cpp(234,15)
  at parse_document() in xml.cpp(567,10)
  at load_config() in main.cpp(89,5)
```

### Message vs Position Policy

- `error.position` is for machines (byte offset).
- Line/column information, when needed, is included in `error.message` for humans.

## Best Practices

### 1. Use Descriptive Messages

```cpp
// Good
unexpected_error(errors::invalid_argument,
  yw::format("Expected number between {} and {}, got {}", min, max, value));

// Less helpful
unexpected_error(errors::invalid_argument, "invalid value");
```

### 2. Preserve Error Information

When propagating errors, don't lose the original error trace:

```cpp
// Good - preserves trace
return unexpected_error(result.error());

// Bad - loses trace
return unexpected_error(errors::operation_failed, "something failed");
```

### 3. Use Position for Parsing

When parsing text, always include the position where the error occurred:

```cpp
return unexpected_error(
  errors::invalid_argument,
  "unexpected character",
  0,  // system_code
  static_cast<uint64_t>(current_pos - input_start)
);
```

### 4. Add System Codes for System Calls

When system APIs fail, capture their error codes:

```cpp
#ifdef _WIN32
  return unexpected_error(errors::invalid_file, "CreateFile failed", GetLastError());
#else
  return unexpected_error(errors::invalid_file, "open failed", errno);
#endif
```

### 5. Let Source Locations Capture Automatically

Don't manually specify source locations unless necessary - the default parameter captures them automatically:

```cpp
// Good - automatic capture
return unexpected_error(errors::operation_failed, "failed");

// Usually unnecessary
return unexpected_error(errors::operation_failed, "failed", 0, -1, source{});
```

## Integration with XML Parser

The XML parser in ywlib demonstrates advanced error handling with position tracking:

```cpp
template<bool View> class comment {
  static constexpr std::expected<comment<View>, error_trace>
  parse(std::string_view& rest, std::string_view doc) {
    if (!rest.starts_with("<!--"sv)) {
      return unexpected_error("xml: expected comment '<!--'", rest.data(), doc);
    }
    // ... parsing logic ...
  }
};
```

The parser tracks:
- Current position in the document
- Line and column numbers
- Context around the error for helpful messages

## Comparison with Exceptions

| Feature | `std::expected` | Exceptions |
|---------|----------------|------------|
| Performance | Zero overhead on success path | Overhead always present |
| Error visibility | Explicit in function signature | Hidden from signature |
| Error handling | Mandatory to check | Can be ignored |
| Stack traces | Manual with `error_trace` | Automatic (but costly) |
| Determinism | Fully deterministic | Non-deterministic unwinding |
| Zero-cost abstraction | Yes (when successful) | No |

## See Also

- `yw/core.h` - Core error types and utilities
- `yw/file.h` - File I/O error handling
- `yw/xml.h` - Example of error handling in a parser
