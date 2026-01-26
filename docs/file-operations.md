# File Operations in ywlib

## Overview

ywlib provides a cross-platform file I/O interface through the `yw::file_handle` class. The API wraps platform-specific file operations (Windows and POSIX) with a modern C++23 interface using `std::expected` for error handling. All operations are exception-free and provide detailed error information.

## Key Features

- **Cross-platform**: Unified API for Windows and POSIX systems
- **Error Handling**: All operations return `std::expected<T, error_trace>` with detailed error information
- **RAII**: Automatic resource management - files are closed on destruction
- **Move Semantics**: `file_handle` is move-only for safe ownership transfer
- **Binary and Text**: Support for both binary and text file operations
- **Large Files**: 64-bit file positions for files larger than 4GB
- **Trivial Types**: Optimized read/write for trivially copyable types

## Core Components

### Open Modes

The `yw::open_mode` enum specifies how to open a file:

```cpp
enum class open_mode {
  unknown,           // Invalid/uninitialized state
  read_existing,     // Open existing file for reading only
  update_existing,   // Open existing file for reading and writing
  create_always,     // Create new file, overwrite if exists
  create_new,        // Create new file, fail if exists
  append,            // Open for appending (create if doesn't exist)
  update_or_create   // Open for reading/writing (create if doesn't exist)
};
```

| Mode | Read | Write | Creates | Truncates | Fails if Exists |
|------|------|-------|---------|-----------|-----------------|
| `read_existing` | ✓ | - | - | - | - |
| `update_existing` | ✓ | ✓ | - | - | - |
| `create_always` | ✓ | ✓ | ✓ | ✓ | - |
| `create_new` | ✓ | ✓ | ✓ | - | ✓ |
| `append` | - | ✓ | ✓ | - | - |
| `update_or_create` | ✓ | ✓ | ✓ | - | - |

### Seek Modes

The `yw::seek_whence` enum specifies the reference point for seeking:

```cpp
enum class seek_whence {
  begin = SEEK_SET,    // Offset from beginning of file
  current = SEEK_CUR,  // Offset from current position
  end = SEEK_END       // Offset from end of file
};
```

### File Handle

The `yw::file_handle` class manages file resources:

```cpp
class file_handle {
public:
  file_handle() noexcept;                                    // Default constructor
  file_handle(const file_handle&) = delete;                 // Non-copyable
  file_handle(file_handle&&) noexcept;                      // Move constructor
  file_handle& operator=(file_handle&&) noexcept;           // Move assignment
  ~file_handle();                                            // Auto-close on destruction

  // Factory method
  static std::expected<file_handle, error_trace> create(
    const std::filesystem::path& path,
    open_mode mode
  );

  // Properties
  const std::filesystem::path& path() const noexcept;
  open_mode mode() const noexcept;
  bool is_open() const noexcept;

  // File operations (all return std::expected)
  std::expected<void, error_trace> close();
  std::expected<int64_t, error_trace> tell() const;
  std::expected<void, error_trace> seek(int64_t offset, seek_whence whence);
  std::expected<int64_t, error_trace> file_size() const;

  // Reading
  std::expected<size_t, error_trace> read(void* dst, size_t bytes);
  std::expected<void, error_trace> read_exact(void* dst, size_t bytes);
  template<trivial T> std::expected<T, error_trace> read_trivial();
  template<trivial T> std::expected<void, error_trace> read_trivial(T& v);

  // Writing
  std::expected<size_t, error_trace> write(const void* src, size_t bytes);
  std::expected<void, error_trace> write_exact(const void* src, size_t bytes);
  template<trivial T> std::expected<void, error_trace> write_trivial(const T& v);

  // Utility
  std::expected<void, error_trace> flush();
  std::expected<void, error_trace> truncate_to_current();
  std::expected<void, error_trace> close_at_current();
};
```

### Helper Function

A convenient `open()` function is provided:

```cpp
std::expected<file_handle, error_trace> yw::open(
  const std::filesystem::path& path,
  open_mode mode
);
```

## Usage Patterns

### Opening Files

#### Basic File Opening

```cpp
#include <ywlib>

// Open for reading
auto file = yw::open("data.bin", yw::open_mode::read_existing);
if (!file) {
  yw::print(stderr, "Failed to open file: {}\n", file.error());
  return 1;
}

// Use the file...
```

#### All Open Modes

```cpp
// Read only
auto reader = yw::open("input.txt", yw::open_mode::read_existing);

// Read and write existing file
auto updater = yw::open("data.db", yw::open_mode::update_existing);

// Create new file (overwrite if exists)
auto creator = yw::open("output.log", yw::open_mode::create_always);

// Create new file (fail if exists)
auto exclusive = yw::open("lock.file", yw::open_mode::create_new);
if (!exclusive) {
  // File already exists
}

// Append to file
auto logger = yw::open("app.log", yw::open_mode::append);

// Read/write, create if doesn't exist
auto rw = yw::open("config.dat", yw::open_mode::update_or_create);
```

#### Using the Factory Method

```cpp
auto file = yw::file_handle::create("data.bin", yw::open_mode::read_existing);
if (!file) {
  return unexpected_error(file.error());
}
// file is of type std::expected<yw::file_handle, yw::error_trace>
```

### Reading Files

#### Reading Arbitrary Data

```cpp
auto file = yw::open("data.bin", yw::open_mode::read_existing);
if (!file) return 1;

// Read up to N bytes (partial read is OK)
std::vector<std::byte> buffer(1024);
auto n_read = file->read(buffer.data(), buffer.size());
if (!n_read) {
  yw::print(stderr, "Read error: {}\n", n_read.error());
  return 1;
}
yw::print("Read {} bytes\n", *n_read);

// Read exactly N bytes (error on partial read)
std::array<std::byte, 256> exact_buffer;
if (auto res = file->read_exact(exact_buffer.data(), exact_buffer.size()); !res) {
  yw::print(stderr, "Failed to read exact amount: {}\n", res.error());
  return 1;
}
```

#### Reading Trivial Types

```cpp
auto file = yw::open("binary.dat", yw::open_mode::read_existing);
if (!file) return 1;

// Read a single value
auto value = file->read_trivial<int32_t>();
if (!value) {
  yw::print(stderr, "Failed to read int32: {}\n", value.error());
  return 1;
}
yw::print("Value: {}\n", *value);

// Read into existing variable
float pi;
if (auto res = file->read_trivial(pi); !res) {
  yw::print(stderr, "Failed to read float: {}\n", res.error());
  return 1;
}

// Read a struct
struct Header {
  uint32_t magic;
  uint32_t version;
  uint64_t timestamp;
};

auto header = file->read_trivial<Header>();
if (!header) {
  yw::print(stderr, "Invalid header: {}\n", header.error());
  return 1;
}
```

#### Reading Entire File

```cpp
std::expected<std::vector<std::byte>, error_trace>
read_entire_file(const std::filesystem::path& path) {
  auto file = yw::open(path, yw::open_mode::read_existing);
  if (!file) {
    return unexpected_error(file.error());
  }

  auto size = file->file_size();
  if (!size) {
    return unexpected_error(size.error());
  }

  std::vector<std::byte> data(*size);
  if (auto res = file->read_exact(data.data(), data.size()); !res) {
    return unexpected_error(res.error());
  }

  return data;
}
```

### Writing Files

#### Writing Arbitrary Data

```cpp
auto file = yw::open("output.bin", yw::open_mode::create_always);
if (!file) return 1;

// Write data (may write less than requested)
const std::string message = "Hello, world!";
auto n_written = file->write(message.data(), message.size());
if (!n_written) {
  yw::print(stderr, "Write error: {}\n", n_written.error());
  return 1;
}
yw::print("Wrote {} bytes\n", *n_written);

// Write exact amount (error on partial write)
std::vector<std::byte> data = get_data();
if (auto res = file->write_exact(data.data(), data.size()); !res) {
  yw::print(stderr, "Failed to write all data: {}\n", res.error());
  return 1;
}

// Flush to disk
if (auto res = file->flush(); !res) {
  yw::print(stderr, "Flush failed: {}\n", res.error());
}
```

#### Writing Trivial Types

```cpp
auto file = yw::open("numbers.dat", yw::open_mode::create_always);
if (!file) return 1;

// Write single values
if (auto res = file->write_trivial(int32_t{42}); !res) {
  return 1;
}

if (auto res = file->write_trivial(3.14159); !res) {
  return 1;
}

// Write a struct
struct Record {
  uint64_t id;
  double value;
  char name[32];
};

Record rec{123, 99.5, "test"};
if (auto res = file->write_trivial(rec); !res) {
  yw::print(stderr, "Failed to write record: {}\n", res.error());
  return 1;
}
```

#### Appending to Files

```cpp
auto log = yw::open("app.log", yw::open_mode::append);
if (!log) return 1;

auto timestamp = std::chrono::system_clock::now();
auto message = yw::format("[{}] Application started\n", timestamp);

if (auto res = log->write_exact(message.data(), message.size()); !res) {
  yw::print(stderr, "Failed to write log: {}\n", res.error());
}

// Flush ensures it's written to disk
log->flush();
```

### Seeking and Position

#### Basic Seeking

```cpp
auto file = yw::open("data.bin", yw::open_mode::update_existing);
if (!file) return 1;

// Get current position
auto pos = file->tell();
if (!pos) {
  yw::print(stderr, "tell() failed: {}\n", pos.error());
  return 1;
}
yw::print("Current position: {}\n", *pos);

// Seek to beginning
if (auto res = file->seek(0, yw::seek_whence::begin); !res) {
  yw::print(stderr, "seek() failed: {}\n", res.error());
  return 1;
}

// Seek to end
file->seek(0, yw::seek_whence::end);

// Seek relative to current position
file->seek(100, yw::seek_whence::current);   // Forward 100 bytes
file->seek(-50, yw::seek_whence::current);   // Back 50 bytes

// Seek to specific position
file->seek(1024, yw::seek_whence::begin);
```

#### Get File Size

```cpp
auto file = yw::open("data.bin", yw::open_mode::read_existing);
if (!file) return 1;

auto size = file->file_size();
if (!size) {
  yw::print(stderr, "Failed to get file size: {}\n", size.error());
  return 1;
}
yw::print("File size: {} bytes\n", *size);
```

#### Random Access

```cpp
std::expected<std::vector<Record>, error_trace>
read_records(const std::filesystem::path& path,
             const std::vector<size_t>& indices) {
  auto file = yw::open(path, yw::open_mode::read_existing);
  if (!file) {
    return unexpected_error(file.error());
  }

  std::vector<Record> records;
  records.reserve(indices.size());

  for (size_t idx : indices) {
    // Seek to record position
    if (auto res = file->seek(idx * sizeof(Record)); !res) {
      return unexpected_error(res.error());
    }

    // Read record
    auto rec = file->read_trivial<Record>();
    if (!rec) {
      return unexpected_error(rec.error());
    }

    records.push_back(*rec);
  }

  return records;
}
```

### Truncating Files

#### Truncate to Current Position

```cpp
auto file = yw::open("data.bin", yw::open_mode::update_existing);
if (!file) return 1;

// Read and validate header
auto header = file->read_trivial<Header>();
if (!header || !is_valid(*header)) {
  yw::print(stderr, "Invalid header, truncating file\n");

  // Rewind and truncate
  file->seek(0);
  if (auto res = file->truncate_to_current(); !res) {
    yw::print(stderr, "Truncate failed: {}\n", res.error());
    return 1;
  }
}
```

#### Close and Truncate

```cpp
// Write partial data and truncate
auto file = yw::open("output.dat", yw::open_mode::create_always);
if (!file) return 1;

// Write some data
file->write_exact(data.data(), 1000);

// Truncate at current position and close
if (auto res = file->close_at_current(); !res) {
  yw::print(stderr, "Failed to close and truncate: {}\n", res.error());
  return 1;
}
// File is now exactly 1000 bytes and closed
```

### Resource Management

#### RAII Automatic Cleanup

```cpp
{
  auto file = yw::open("temp.dat", yw::open_mode::create_always);
  if (!file) return 1;

  // Use file...
  file->write_exact(data.data(), data.size());

  // File is automatically closed when 'file' goes out of scope
}
// File is closed here
```

#### Manual Close

```cpp
auto file = yw::open("data.bin", yw::open_mode::update_existing);
if (!file) return 1;

// Do work...

// Explicitly close and check for errors
if (auto res = file->close(); !res) {
  yw::print(stderr, "Warning: close() failed: {}\n", res.error());
}

// Can check if closed
if (!file->is_open()) {
  yw::print("File is closed\n");
}
```

#### Move Semantics

```cpp
std::expected<file_handle, error_trace>
open_config_file() {
  auto file = yw::open("config.ini", yw::open_mode::read_existing);
  if (!file) {
    return unexpected_error(file.error());
  }

  // Move ownership to caller
  return std::move(*file);
}

// Usage
auto config = open_config_file();
if (!config) {
  yw::print(stderr, "Failed to open config: {}\n", config.error());
  return 1;
}
// 'config' now owns the file handle
```

## Complete Examples

### Binary File Format Reader/Writer

```cpp
#include <ywlib>

struct FileHeader {
  uint32_t magic;
  uint32_t version;
  uint32_t record_count;
  uint32_t reserved;
};

struct Record {
  uint64_t id;
  double value;
  char name[32];
};

constexpr uint32_t MAGIC = 0x52454344; // "RECR"
constexpr uint32_t VERSION = 1;

std::expected<void, error_trace>
write_records(const std::filesystem::path& path,
              const std::vector<Record>& records) {
  auto file = yw::open(path, yw::open_mode::create_always);
  if (!file) {
    return unexpected_error(file.error());
  }

  // Write header
  FileHeader header{
    .magic = MAGIC,
    .version = VERSION,
    .record_count = static_cast<uint32_t>(records.size()),
    .reserved = 0
  };

  if (auto res = file->write_trivial(header); !res) {
    return unexpected_error(res.error());
  }

  // Write records
  for (const auto& rec : records) {
    if (auto res = file->write_trivial(rec); !res) {
      return unexpected_error(res.error());
    }
  }

  // Ensure data is written
  if (auto res = file->flush(); !res) {
    return unexpected_error(res.error());
  }

  return {};
}

std::expected<std::vector<Record>, error_trace>
read_records(const std::filesystem::path& path) {
  auto file = yw::open(path, yw::open_mode::read_existing);
  if (!file) {
    return unexpected_error(file.error());
  }

  // Read and validate header
  auto header = file->read_trivial<FileHeader>();
  if (!header) {
    return unexpected_error(header.error());
  }

  if (header->magic != MAGIC) {
    return unexpected_error(
      errors::invalid_file,
      yw::format("Invalid magic number: {:#x}", header->magic)
    );
  }

  if (header->version != VERSION) {
    return unexpected_error(
      errors::invalid_file,
      yw::format("Unsupported version: {}", header->version)
    );
  }

  // Read records
  std::vector<Record> records;
  records.reserve(header->record_count);

  for (uint32_t i = 0; i < header->record_count; ++i) {
    auto rec = file->read_trivial<Record>();
    if (!rec) {
      return unexpected_error(
        errors::invalid_file,
        yw::format("Failed to read record {}", i)
      );
    }
    records.push_back(*rec);
  }

  return records;
}

int main() {
  // Create sample records
  std::vector<Record> records = {
    {1, 3.14, "pi"},
    {2, 2.718, "e"},
    {3, 1.414, "sqrt2"}
  };

  // Write to file
  if (auto res = write_records("records.dat", records); !res) {
    yw::print(stderr, "Write failed: {}\n", res.error());
    return 1;
  }

  // Read back
  auto loaded = read_records("records.dat");
  if (!loaded) {
    yw::print(stderr, "Read failed: {}\n", loaded.error());
    return 1;
  }

  // Display
  yw::print("Loaded {} records:\n", loaded->size());
  for (const auto& rec : *loaded) {
    yw::print("  {} | {} | {}\n", rec.id, rec.value, rec.name);
  }

  return 0;
}
```

### Text File Processing

```cpp
#include <ywlib>

std::expected<std::vector<std::string>, error_trace>
read_lines(const std::filesystem::path& path) {
  auto file = yw::open(path, yw::open_mode::read_existing);
  if (!file) {
    return unexpected_error(file.error());
  }

  auto size = file->file_size();
  if (!size) {
    return unexpected_error(size.error());
  }

  std::string content(*size, '\0');
  if (auto res = file->read_exact(content.data(), content.size()); !res) {
    return unexpected_error(res.error());
  }

  std::vector<std::string> lines;
  std::string_view view = content;

  while (!view.empty()) {
    auto pos = view.find('\n');
    if (pos == std::string_view::npos) {
      lines.emplace_back(view);
      break;
    }
    lines.emplace_back(view.substr(0, pos));
    view.remove_prefix(pos + 1);
  }

  return lines;
}

std::expected<void, error_trace>
write_lines(const std::filesystem::path& path,
            const std::vector<std::string>& lines) {
  auto file = yw::open(path, yw::open_mode::create_always);
  if (!file) {
    return unexpected_error(file.error());
  }

  for (const auto& line : lines) {
    if (auto res = file->write_exact(line.data(), line.size()); !res) {
      return unexpected_error(res.error());
    }

    if (auto res = file->write_exact("\n", 1); !res) {
      return unexpected_error(res.error());
    }
  }

  return {};
}
```

### Memory-Mapped Alternative (Manual)

```cpp
// Note: ywlib doesn't provide memory mapping directly,
// but you can combine file_handle with platform APIs

#ifdef _WIN32
#include <windows.h>

std::expected<std::span<const std::byte>, error_trace>
map_file_readonly(const std::filesystem::path& path) {
  auto file = yw::open(path, yw::open_mode::read_existing);
  if (!file) {
    return unexpected_error(file.error());
  }

  auto size = file->file_size();
  if (!size) {
    return unexpected_error(size.error());
  }

  // Get file handle for mapping
  // Note: This is a simplified example
  // In production, you'd need to extract the native handle

  HANDLE hFile = /* ... extract from file ... */;
  HANDLE hMapping = CreateFileMappingW(hFile, nullptr, PAGE_READONLY,
                                        0, 0, nullptr);
  if (!hMapping) {
    return unexpected_error(errors::operation_failed,
                           "CreateFileMapping failed",
                           GetLastError());
  }

  void* mapped = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
  if (!mapped) {
    CloseHandle(hMapping);
    return unexpected_error(errors::operation_failed,
                           "MapViewOfFile failed",
                           GetLastError());
  }

  return std::span<const std::byte>(
    static_cast<const std::byte*>(mapped),
    static_cast<size_t>(*size)
  );
}
#endif
```

## Platform-Specific Behavior

### Windows

On Windows, file operations use:
- `CreateFileW()` for opening files (Unicode support)
- `_open_osfhandle()` to get a C file descriptor
- `_fdopen()` to get a `FILE*` handle
- `_fseeki64()` / `_ftelli64()` for 64-bit seeking
- `_chsize_s()` for file truncation

All file paths are converted to UTF-16 for Windows APIs, ensuring proper Unicode support.

### POSIX (Linux/macOS/Unix)

On POSIX systems:
- `open()` with appropriate flags
- `fdopen()` to get a `FILE*` handle
- `fseeko()` / `ftello()` for 64-bit seeking (where available)
- `ftruncate()` for file truncation

File paths are passed as-is (typically UTF-8).

## Error Handling

All operations return `std::expected<T, error_trace>`. Common error scenarios:

### File Not Found

```cpp
auto file = yw::open("nonexistent.txt", yw::open_mode::read_existing);
if (!file) {
  // error.error.code == errors::operation_failed
  // error.error.system_code == ENOENT (POSIX) or ERROR_FILE_NOT_FOUND (Windows)
  yw::print(stderr, "{}\n", file.error());
}
```

### Permission Denied

```cpp
auto file = yw::open("/root/secret.txt", yw::open_mode::read_existing);
if (!file) {
  // error.error.system_code == EACCES (POSIX) or ERROR_ACCESS_DENIED (Windows)
  yw::print(stderr, "Permission denied: {}\n", file.error());
}
```

### File Already Exists

```cpp
auto file = yw::open("existing.txt", yw::open_mode::create_new);
if (!file) {
  // error.error.system_code == EEXIST (POSIX) or ERROR_FILE_EXISTS (Windows)
  yw::print(stderr, "File exists: {}\n", file.error());
}
```

### Disk Full

```cpp
auto file = yw::open("output.txt", yw::open_mode::create_always);
if (!file) return 1;

std::vector<std::byte> large_data(1000000000); // 1GB
auto res = file->write_exact(large_data.data(), large_data.size());
if (!res) {
  // error.error.system_code == ENOSPC (POSIX) or ERROR_DISK_FULL (Windows)
  yw::print(stderr, "Disk full: {}\n", res.error());
}
```

### Unexpected EOF

```cpp
auto file = yw::open("data.bin", yw::open_mode::read_existing);
if (!file) return 1;

std::array<std::byte, 10000> buffer;
auto res = file->read_exact(buffer.data(), buffer.size());
if (!res) {
  // File is smaller than expected
  yw::print(stderr, "Unexpected EOF: {}\n", res.error());
}
```

## Best Practices

### 1. Always Check Return Values

```cpp
// Good
auto file = yw::open(path, mode);
if (!file) {
  handle_error(file.error());
  return;
}

// Bad - ignoring errors
auto file = yw::open(path, mode);
file->write_exact(data, size); // Might crash if file is invalid!
```

### 2. Use RAII for Automatic Cleanup

```cpp
// Good - automatic cleanup
{
  auto file = yw::open(path, mode);
  if (!file) return;

  // Use file...

} // Automatically closed

// Avoid manual close unless you need error checking
auto file = yw::open(path, mode);
// ...
if (auto res = file->close(); !res) {
  // Only if you need to handle close errors
}
```

### 3. Prefer read_exact/write_exact for Fixed-Size Data

```cpp
// Good - ensures all data is read
std::array<std::byte, 1024> buffer;
if (auto res = file->read_exact(buffer.data(), buffer.size()); !res) {
  // Error: couldn't read full amount
}

// Avoid for fixed-size reads
auto n = file->read(buffer.data(), buffer.size());
if (n && *n < buffer.size()) {
  // Partial read - might be unexpected
}
```

### 4. Flush Important Data

```cpp
// For critical data (logs, databases)
auto log = yw::open("critical.log", yw::open_mode::append);
log->write_exact(message.data(), message.size());
log->flush(); // Ensure it reaches disk

// For performance (batch writes)
for (const auto& record : many_records) {
  file->write_exact(&record, sizeof(record));
}
file->flush(); // Flush once at the end
```

### 5. Check File Size Before Reading

```cpp
// Good
auto file = yw::open(path, yw::open_mode::read_existing);
if (!file) return;

auto size = file->file_size();
if (!size) return;

if (*size > MAX_FILE_SIZE) {
  yw::print(stderr, "File too large: {} bytes\n", *size);
  return;
}

std::vector<std::byte> data(*size);
file->read_exact(data.data(), data.size());
```

### 6. Use Appropriate Open Modes

```cpp
// Reading configuration - read_existing
auto config = yw::open("config.ini", yw::open_mode::read_existing);

// Logging - append
auto log = yw::open("app.log", yw::open_mode::append);

// Temporary output - create_always
auto temp = yw::open("temp.dat", yw::open_mode::create_always);

// Database updates - update_or_create
auto db = yw::open("data.db", yw::open_mode::update_or_create);

// Lock files - create_new (fail if exists)
auto lock = yw::open("app.lock", yw::open_mode::create_new);
if (!lock) {
  yw::print("Application already running\n");
  return 1;
}
```

### 7. Handle Large Files Carefully

```cpp
// Good - process in chunks
auto file = yw::open(large_file, yw::open_mode::read_existing);
if (!file) return;

constexpr size_t CHUNK_SIZE = 1024 * 1024; // 1MB
std::vector<std::byte> chunk(CHUNK_SIZE);

while (true) {
  auto n = file->read(chunk.data(), chunk.size());
  if (!n) {
    yw::print(stderr, "Read error: {}\n", n.error());
    break;
  }

  if (*n == 0) break; // EOF

  process_chunk(std::span(chunk.data(), *n));
}

// Avoid - loading entire file
auto size = file->file_size();
std::vector<std::byte> all_data(*size); // Might fail for very large files
file->read_exact(all_data.data(), all_data.size());
```

## Performance Considerations

### Buffering

All file operations use `FILE*` which provides buffering by default. For most use cases, this is optimal.

### Trivial Type Optimization

`read_trivial<T>()` and `write_trivial<T>()` are optimized for types that are trivially copyable:

```cpp
// Efficient - single fread/fwrite
struct Point { double x, y, z; };
auto pt = file->read_trivial<Point>();

// Less efficient - multiple calls
Point pt;
file->read_exact(&pt.x, sizeof(double));
file->read_exact(&pt.y, sizeof(double));
file->read_exact(&pt.z, sizeof(double));
```

### Sequential Access

For sequential reads/writes, the default buffering is very efficient:

```cpp
// Good - sequential access benefits from buffering
for (int i = 0; i < 1000; ++i) {
  file->write_trivial(data[i]);
}
```

### Random Access

For random access, consider reading larger chunks:

```cpp
// Less efficient - many small seeks
for (auto idx : indices) {
  file->seek(idx * sizeof(Record));
  auto rec = file->read_trivial<Record>();
}

// More efficient - read larger blocks
file->seek(first_idx * sizeof(Record));
std::vector<Record> records(count);
file->read_exact(records.data(), records.size() * sizeof(Record));
```

## Comparison with Other Libraries

| Feature | ywlib | `std::fstream` | POSIX `open()` | Windows API |
|---------|-------|----------------|----------------|-------------|
| Cross-platform | ✓ | ✓ | - | - |
| Error handling | `std::expected` | exceptions/flags | error codes | error codes |
| Unicode (Windows) | ✓ | Partial | - | ✓ |
| 64-bit files | ✓ | ✓ | ✓ | ✓ |
| Binary I/O | ✓ | ✓ | ✓ | ✓ |
| Type safety | ✓ | ✓ | - | - |
| RAII | ✓ | ✓ | - | - |
| Modern C++ | C++23 | C++98 | C | - |

## See Also

- [yw/file.h](../ywlib/yw/file.h) - Source code with full implementation
- [yw/core.h](../ywlib/yw/core.h) - Core utilities and error handling
- [error-handling.md](error-handling.md) - Detailed error handling documentation
