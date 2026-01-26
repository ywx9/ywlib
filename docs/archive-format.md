# Archive Format in ywlib

## Overview

ywlib provides a simple, uncompressed archive format (`.ywa` files) through the `yw::archive` module. The format is designed for simplicity and reliability, featuring footer-based indexing, CRC32 integrity checking, and full Unicode support for entry names.

## Key Features

- **Simple Format**: Uncompressed, straightforward binary structure
- **Footer-Based Indexing**: Fast lookup without scanning entire file
- **Integrity Checking**: CRC32 checksums (IEEE 802.3) for each entry
- **Unicode Support**: UTF-8 entry names up to 2048 bytes
- **Cross-Platform**: Little-endian format works on all platforms
- **No Compression**: Prioritizes simplicity and access speed over size
- **RAII**: Automatic resource management with move-only semantics

## File Format Specification

### Overview

A `.ywa` archive consists of:
1. **Entries** (0 or more): Each containing magic, metadata, name, data, and CRC32
2. **Footer**: Index containing entry offsets and archive metadata

### Binary Structure

```
┌────────────────────────────────────────────────┐
│ Entry 0                                        │
│  ┌─────────────────────────────────────────┐   │
│  │ Magic (4 bytes)      : 0x45415759       │   │
│  │ Name Length (4)      : uint32_t         │   │
│  │ Data Length (8)      : uint64_t         │   │
│  │ Name (variable)      : UTF-8 string     │   │
│  │ Data (variable)      : raw bytes        │   │
│  │ CRC32 (4)            : uint32_t         │   │
│  └─────────────────────────────────────────┘   │
├────────────────────────────────────────────────┤
│ Entry 1 ... Entry N-1                          │
│ (same structure as Entry 0)                    │
├────────────────────────────────────────────────┤
│ Footer                                         │
│  ┌─────────────────────────────────────────┐   │
│  │ Magic (4 bytes)      : 0x46415759       │   │
│  │ Entry Count (4)      : uint32_t = N     │   │
│  │ Entry Offsets (8×N)  : uint64_t[N]      │   │
│  │ Footer Offset (8)    : uint64_t         │   │
│  └─────────────────────────────────────────┘   │
└────────────────────────────────────────────────┘
```

### Entry Structure

Each entry has the following structure:

| Offset | Size | Type | Description |
|--------|------|------|-------------|
| 0 | 4 | `uint32_t` | Magic number: `0x45415759` ('YWAE') |
| 4 | 4 | `uint32_t` | Name length in bytes (1-2048) |
| 8 | 8 | `uint64_t` | Data length in bytes |
| 16 | variable | `char[]` | UTF-8 encoded entry name |
| 16+N | variable | `byte[]` | Entry data |
| 16+N+D | 4 | `uint32_t` | CRC32 checksum of data |

**Entry Magic**: `YWAE` (0x45415759 in little-endian)

### Footer Structure

The footer appears at the end of the file:

| Offset | Size | Type | Description |
|--------|------|------|-------------|
| 0 | 4 | `uint32_t` | Magic number: `0x46415759` ('YWAF') |
| 4 | 4 | `uint32_t` | Number of entries (N) |
| 8 | 8×N | `uint64_t[N]` | File offset of each entry |
| 8+8×N | 8 | `uint64_t` | File offset of this footer |

**Footer Magic**: `YWAF` (0x46415759 in little-endian)

### Format Rules

1. **Endianness**: All numeric fields are little-endian
2. **No Padding**: No padding between entries or between entries and footer
3. **Entry Names**: UTF-8 strings, 1-2048 bytes, not null-terminated
4. **Entry Order**: Footer offsets correspond to entry order
5. **CRC32 Algorithm**: IEEE 802.3 standard
   - Polynomial: `0x04C11DB7`
   - Initial value: `0xFFFFFFFF`
   - Reflect input: `true`
   - Reflect output: `true`
   - XOR output: `0xFFFFFFFF`
   - Test vector: "123456789" → `0xCBF43926`

### Example Archive

A minimal archive with one entry named "test.txt" containing "Hello":

```
Offset | Hex Data                    | Description
-------|-----------------------------|------------------
0x0000 | 59 57 41 45                 | Entry magic (YWAE)
0x0004 | 08 00 00 00                 | Name length = 8
0x0008 | 05 00 00 00 00 00 00 00     | Data length = 5
0x0010 | 74 65 73 74 2E 74 78 74     | Name: "test.txt"
0x0018 | 48 65 6C 6C 6F              | Data: "Hello"
0x001D | [CRC32 of "Hello"]          | CRC32 checksum
0x0021 | 59 57 41 46                 | Footer magic (YWAF)
0x0025 | 01 00 00 00                 | Entry count = 1
0x0029 | 00 00 00 00 00 00 00 00     | Entry 0 offset = 0
0x0031 | 21 00 00 00 00 00 00 00     | Footer offset = 0x21
```

## Core Components

### Entry Structure

```cpp
struct entry {
  std::string name;          // UTF-8 entry name
  uint64_t entry_offset;     // Offset to entry header in file
  uint64_t data_offset;      // Offset to data in file
  uint64_t data_length;      // Size of data in bytes
  uint32_t crc32;            // CRC32 checksum of data
};
```

### Archive Handle

```cpp
class archive::handle {
public:
  // Factory method
  static std::expected<handle, error_trace> create(
    const std::filesystem::path& path,
    open_mode mode
  );

  // Properties
  bool is_open() const noexcept;
  const std::filesystem::path& path() const noexcept;
  open_mode mode() const noexcept;
  const std::vector<entry>& entries() const noexcept;

  // Operations
  std::expected<void, error_trace> close();
  std::expected<void, error_trace> flush();

  // Reading
  std::expected<std::vector<std::byte>, error_trace> read(size_t index);
  std::expected<std::vector<std::byte>, error_trace> read(const std::string& name);

  // Verification
  std::expected<bool, error_trace> verify(size_t index);
  std::expected<bool, error_trace> verify(const std::string& name);

  // Writing
  std::expected<void, error_trace> append(
    const std::string& name,
    const void* data,
    size_t data_length
  );
  std::expected<void, error_trace> remove(size_t n);

  // RAII
  ~handle(); // Auto-close and flush
  handle(handle&&) noexcept;
  handle& operator=(handle&&) noexcept;
};
```

### Helper Functions

```cpp
// Open an archive
std::expected<handle, error_trace> yw::archive::open(
  const std::filesystem::path& path,
  open_mode mode
);

// Pack directory into archive
std::expected<void, error_trace> yw::archive::pack(
  const std::filesystem::path& src_path,
  const std::filesystem::path& dst_path,
  open_mode mode = open_mode::create_always
);

// Extract archive to directory
std::expected<void, error_trace> yw::archive::extract(
  const std::filesystem::path& src_path,
  const std::filesystem::path& dst_path
);
```

## Usage Patterns

### Creating Archives

#### Create Empty Archive

```cpp
#include <ywlib>

// Create new archive
auto archive = yw::archive::open("data.ywa", yw::open_mode::create_always);
if (!archive) {
  yw::print(stderr, "Failed to create archive: {}\n", archive.error());
  return 1;
}

// Archive is ready for appending entries
```

#### Add Entries

```cpp
auto archive = yw::archive::open("data.ywa", yw::open_mode::create_always);
if (!archive) return 1;

// Add text entry
std::string text = "Hello, World!";
if (auto res = archive->append("hello.txt", text.data(), text.size()); !res) {
  yw::print(stderr, "Failed to append: {}\n", res.error());
  return 1;
}

// Add binary entry
std::vector<std::byte> data = load_binary_data();
if (auto res = archive->append("data.bin", data.data(), data.size()); !res) {
  yw::print(stderr, "Failed to append: {}\n", res.error());
  return 1;
}

// Archive is automatically flushed and closed on destruction
```

#### Pack Entire Directory

```cpp
// Pack all files from src_dir/ into archive.ywa
auto result = yw::archive::pack("src_dir", "archive.ywa");
if (!result) {
  yw::print(stderr, "Failed to pack: {}\n", result.error());
  return 1;
}

yw::print("Successfully packed directory\n");
```

### Reading Archives

#### Open and List Entries

```cpp
auto archive = yw::archive::open("data.ywa", yw::open_mode::read_existing);
if (!archive) {
  yw::print(stderr, "Failed to open: {}\n", archive.error());
  return 1;
}

yw::print("Archive contains {} entries:\n", archive->entries().size());
for (const auto& entry : archive->entries()) {
  yw::print("  {} ({} bytes, CRC32: {:#010x})\n",
            entry.name, entry.data_length, entry.crc32);
}
```

#### Read Entry by Name

```cpp
auto archive = yw::archive::open("data.ywa", yw::open_mode::read_existing);
if (!archive) return 1;

auto data = archive->read("hello.txt");
if (!data) {
  yw::print(stderr, "Failed to read entry: {}\n", data.error());
  return 1;
}

// Convert to string
std::string text(reinterpret_cast<const char*>(data->data()), data->size());
yw::print("Content: {}\n", text);
```

#### Read Entry by Index

```cpp
auto archive = yw::archive::open("data.ywa", yw::open_mode::read_existing);
if (!archive) return 1;

if (archive->entries().empty()) {
  yw::print("Archive is empty\n");
  return 0;
}

// Read first entry
auto data = archive->read(0);
if (!data) {
  yw::print(stderr, "Failed to read: {}\n", data.error());
  return 1;
}

yw::print("Read {} bytes from {}\n",
          data->size(), archive->entries()[0].name);
```

#### Extract Entire Archive

```cpp
// Extract all files to dst_dir/
auto result = yw::archive::extract("archive.ywa", "dst_dir");
if (!result) {
  yw::print(stderr, "Failed to extract: {}\n", result.error());
  return 1;
}

yw::print("Successfully extracted archive\n");
```

### Verifying Integrity

#### Verify Single Entry

```cpp
auto archive = yw::archive::open("data.ywa", yw::open_mode::read_existing);
if (!archive) return 1;

// Verify by name
auto is_valid = archive->verify("important.dat");
if (!is_valid) {
  yw::print(stderr, "Verification failed: {}\n", is_valid.error());
  return 1;
}

if (*is_valid) {
  yw::print("Entry is valid\n");
} else {
  yw::print("Entry is corrupted!\n");
}
```

#### Verify All Entries

```cpp
auto archive = yw::archive::open("data.ywa", yw::open_mode::read_existing);
if (!archive) return 1;

bool all_valid = true;
for (size_t i = 0; i < archive->entries().size(); ++i) {
  auto is_valid = archive->verify(i);
  if (!is_valid) {
    yw::print(stderr, "Error verifying entry {}: {}\n",
              i, is_valid.error());
    return 1;
  }

  if (!*is_valid) {
    yw::print("Entry {} ({}) is corrupted!\n",
              i, archive->entries()[i].name);
    all_valid = false;
  }
}

if (all_valid) {
  yw::print("All entries verified successfully\n");
}
```

### Updating Archives

#### Append to Existing Archive

```cpp
// Open for updating
auto archive = yw::archive::open("data.ywa", yw::open_mode::update_or_create);
if (!archive) return 1;

yw::print("Archive has {} entries\n", archive->entries().size());

// Add new entry
std::string new_data = "New content";
if (auto res = archive->append("new_file.txt", new_data.data(), new_data.size()); !res) {
  yw::print(stderr, "Failed to append: {}\n", res.error());
  return 1;
}

// Flush writes footer
if (auto res = archive->flush(); !res) {
  yw::print(stderr, "Failed to flush: {}\n", res.error());
  return 1;
}

yw::print("Now has {} entries\n", archive->entries().size());
```

#### Remove Last Entries

```cpp
auto archive = yw::archive::open("data.ywa", yw::open_mode::update_existing);
if (!archive) return 1;

yw::print("Original entry count: {}\n", archive->entries().size());

// Remove last 3 entries
if (auto res = archive->remove(3); !res) {
  yw::print(stderr, "Failed to remove: {}\n", res.error());
  return 1;
}

// Flush to write updated footer
archive->flush();

yw::print("New entry count: {}\n", archive->entries().size());
```

#### Clear All Entries

```cpp
auto archive = yw::archive::open("data.ywa", yw::open_mode::update_existing);
if (!archive) return 1;

// Remove all entries (npos = all)
if (auto res = archive->remove(yw::npos); !res) {
  yw::print(stderr, "Failed to clear: {}\n", res.error());
  return 1;
}

archive->flush();
yw::print("Archive cleared\n");
```

## Complete Examples

### Archive Builder

```cpp
#include <ywlib>

int main(int argc, char** argv) {
  if (auto r = yw::args::parse(argc, argv); !r) {
    yw::print(stderr, "Parse error: {}\n", r.error());
    return 1;
  }

  if (yw::args::positionals.size() < 2) {
    yw::print("Usage: {} <output.ywa> <file1> [file2] ...\n",
              yw::args::program_name);
    return 1;
  }

  const auto output_path = yw::args::positionals[0];

  auto archive = yw::archive::open(output_path, yw::open_mode::create_always);
  if (!archive) {
    yw::print(stderr, "Failed to create archive: {}\n", archive.error());
    return 1;
  }

  // Add each file
  for (size_t i = 1; i < yw::args::positionals.size(); ++i) {
    const auto& file_path = yw::args::positionals[i];

    // Read file
    auto file = yw::open(file_path, yw::open_mode::read_existing);
    if (!file) {
      yw::print(stderr, "Failed to open {}: {}\n",
                file_path, file.error());
      continue;
    }

    auto size = file->file_size();
    if (!size) {
      yw::print(stderr, "Failed to get size of {}: {}\n",
                file_path, size.error());
      continue;
    }

    std::vector<std::byte> data(*size);
    if (auto res = file->read_exact(data.data(), data.size()); !res) {
      yw::print(stderr, "Failed to read {}: {}\n",
                file_path, res.error());
      continue;
    }

    // Add to archive
    if (auto res = archive->append(file_path, data.data(), data.size()); !res) {
      yw::print(stderr, "Failed to append {}: {}\n",
                file_path, res.error());
      return 1;
    }

    yw::print("Added: {} ({} bytes)\n", file_path, data.size());
  }

  // Close archive (auto-flush)
  if (auto res = archive->close(); !res) {
    yw::print(stderr, "Failed to close archive: {}\n", res.error());
    return 1;
  }

  yw::print("Archive created successfully: {}\n", output_path);
  return 0;
}
```

### Archive Extractor

```cpp
#include <ywlib>

int main(int argc, char** argv) {
  if (auto r = yw::args::parse(argc, argv); !r) {
    yw::print(stderr, "Parse error: {}\n", r.error());
    return 1;
  }

  if (yw::args::positionals.size() < 1) {
    yw::print("Usage: {} <archive.ywa> [output_dir]\n",
              yw::args::program_name);
    return 1;
  }

  const auto archive_path = yw::args::positionals[0];
  const auto output_dir = yw::args::positionals.size() > 1
    ? yw::args::positionals[1]
    : ".";

  // Open archive
  auto archive = yw::archive::open(archive_path, yw::open_mode::read_existing);
  if (!archive) {
    yw::print(stderr, "Failed to open archive: {}\n", archive.error());
    return 1;
  }

  yw::print("Extracting {} entries to {}/\n",
            archive->entries().size(), output_dir);

  // Extract each entry
  for (size_t i = 0; i < archive->entries().size(); ++i) {
    const auto& entry = archive->entries()[i];

    // Verify integrity
    auto is_valid = archive->verify(i);
    if (!is_valid) {
      yw::print(stderr, "Failed to verify {}: {}\n",
                entry.name, is_valid.error());
      continue;
    }

    if (!*is_valid) {
      yw::print(stderr, "Warning: {} is corrupted (CRC mismatch)\n",
                entry.name);
      if (!yw::args::has("--force")) {
        yw::print("Skipping (use --force to extract anyway)\n");
        continue;
      }
    }

    // Read data
    auto data = archive->read(i);
    if (!data) {
      yw::print(stderr, "Failed to read {}: {}\n",
                entry.name, data.error());
      continue;
    }

    // Create output path
    std::filesystem::path out_path = output_dir;
    out_path /= yw::unicode<char>(entry.name);

    // Create directories
    std::filesystem::create_directories(out_path.parent_path());

    // Write file
    auto file = yw::open(out_path, yw::open_mode::create_always);
    if (!file) {
      yw::print(stderr, "Failed to create {}: {}\n",
                out_path, file.error());
      continue;
    }

    if (auto res = file->write_exact(data->data(), data->size()); !res) {
      yw::print(stderr, "Failed to write {}: {}\n",
                out_path, res.error());
      continue;
    }

    yw::print("  {} ({} bytes)\n", entry.name, data->size());
  }

  yw::print("Extraction complete\n");
  return 0;
}
```

### Archive Inspector

```cpp
#include <ywlib>

void print_size(uint64_t bytes) {
  if (bytes < 1024) {
    yw::print("{} B", bytes);
  } else if (bytes < 1024 * 1024) {
    yw::print("{:.2f} KB", bytes / 1024.0);
  } else if (bytes < 1024 * 1024 * 1024) {
    yw::print("{:.2f} MB", bytes / (1024.0 * 1024.0));
  } else {
    yw::print("{:.2f} GB", bytes / (1024.0 * 1024.0 * 1024.0));
  }
}

int main(int argc, char** argv) {
  if (auto r = yw::args::parse(argc, argv); !r) {
    yw::print(stderr, "Parse error: {}\n", r.error());
    return 1;
  }

  if (yw::args::positionals.empty()) {
    yw::print("Usage: {} <archive.ywa> [--verify]\n",
              yw::args::program_name);
    return 1;
  }

  const auto archive_path = yw::args::positionals[0];
  const bool verify = yw::args::has("--verify");

  auto archive = yw::archive::open(archive_path, yw::open_mode::read_existing);
  if (!archive) {
    yw::print(stderr, "Failed to open: {}\n", archive.error());
    return 1;
  }

  yw::print("Archive: {}\n", archive_path);
  yw::print("Entries: {}\n\n", archive->entries().size());

  uint64_t total_size = 0;

  yw::print("{:<4} {:<40} {:>12} {:>12} {}\n",
            "#", "Name", "Size", "CRC32", verify ? "Status" : "");
  yw::print("{:-<80}\n", "");

  for (size_t i = 0; i < archive->entries().size(); ++i) {
    const auto& entry = archive->entries()[i];
    total_size += entry.data_length;

    yw::print("{:<4} {:<40} ", i, entry.name);
    print_size(entry.data_length);
    yw::print(" {:>12x}", entry.crc32);

    if (verify) {
      auto is_valid = archive->verify(i);
      if (!is_valid) {
        yw::print(" ERROR");
      } else if (*is_valid) {
        yw::print(" OK");
      } else {
        yw::print(" CORRUPTED");
      }
    }

    yw::print("\n");
  }

  yw::print("{:-<80}\n", "");
  yw::print("Total: ");
  print_size(total_size);
  yw::print("\n");

  return 0;
}
```

## Implementation Details

### CRC32 Algorithm

ywlib uses the IEEE 802.3 CRC32 algorithm:

```cpp
uint32_t calculate_crc32(const std::byte* data, size_t length) {
  uint32_t crc = 0xFFFFFFFF;

  for (size_t i = 0; i < length; ++i) {
    crc ^= static_cast<uint8_t>(data[i]);
    for (int j = 0; j < 8; ++j) {
      crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320 : (crc >> 1);
    }
  }

  return crc ^ 0xFFFFFFFF;
}
```

**Polynomial**: `0xEDB88320` (reversed `0x04C11DB7`)
**Test**: `crc32("123456789")` should equal `0xCBF43926`

### Endianness Handling

All multi-byte values are stored in little-endian format:

```cpp
template<integral T>
constexpr T to_little_endian(T value) noexcept {
  if constexpr (std::endian::native == std::endian::little) {
    return value;
  } else {
    return std::byteswap(value);
  }
}
```

On big-endian systems, values are automatically byte-swapped.

### Footer Location

The last 8 bytes of the file always contain the footer offset:

```cpp
// To find footer:
// 1. Seek to file_size - 8
// 2. Read uint64_t (little-endian)
// 3. This is the footer offset
// 4. Seek to footer offset
// 5. Read footer structure
```

This allows quick access to the index without scanning the entire file.

### Archive Validation

When opening an archive, the following validations are performed:

1. **Footer offset** is within file bounds
2. **Footer magic** is `0x46415759` (YWAF)
3. **Entry count** matches calculated footer size
4. **Entry offsets** are all before footer
5. For each entry:
   - **Entry magic** is `0x45415759` (YWAE)
   - **Name length** is 1-2048 bytes
   - **Entry size** doesn't exceed footer offset
   - **Entry position** matches expected position (no gaps/overlaps)

Any validation failure results in an error with position information.

## Best Practices

### 1. Always Verify on Extract

```cpp
// Good - verify before extracting
auto is_valid = archive->verify(i);
if (!is_valid || !*is_valid) {
  yw::print(stderr, "Entry {} is corrupted\n", i);
  continue;
}
auto data = archive->read(i);

// Risky - extract without verification
auto data = archive->read(i); // Might be corrupted
```

### 2. Flush After Modifications

```cpp
// Good - explicit flush
archive->append("file.txt", data, size);
if (auto res = archive->flush(); !res) {
  handle_error(res.error());
}

// OK - automatic flush on close/destroy
archive->append("file.txt", data, size);
// Flushed automatically when archive goes out of scope
```

### 3. Use Pack/Extract for Directories

```cpp
// Good - simple and correct
yw::archive::pack("src_dir", "archive.ywa");

// Avoid - manual implementation is error-prone
auto archive = yw::archive::open("archive.ywa", yw::open_mode::create_always);
for (const auto& entry : std::filesystem::recursive_directory_iterator("src_dir")) {
  // ... manual file reading and appending
}
```

### 4. Handle Errors Properly

```cpp
// Good - check all operations
auto archive = yw::archive::open(path, mode);
if (!archive) {
  yw::print(stderr, "Open failed: {}\n", archive.error());
  return 1;
}

auto data = archive->read(name);
if (!data) {
  yw::print(stderr, "Read failed: {}\n", data.error());
  return 1;
}

// Bad - ignoring errors
auto archive = yw::archive::open(path, mode);
auto data = archive->read(name); // Crash if archive failed to open!
```

### 5. Use Appropriate Open Modes

```cpp
// Reading only - read_existing
auto archive = yw::archive::open(path, yw::open_mode::read_existing);

// Creating new - create_always
auto archive = yw::archive::open(path, yw::open_mode::create_always);

// Appending - update_or_create
auto archive = yw::archive::open(path, yw::open_mode::update_or_create);

// Modifying - update_existing
auto archive = yw::archive::open(path, yw::open_mode::update_existing);
```

### 6. Be Careful with Entry Names

```cpp
// Good - use relative paths
archive->append("data/config.ini", data, size);

// Avoid - absolute paths may cause issues on extraction
archive->append("/home/user/data/config.ini", data, size);

// Avoid - very long names (limit is 2048 bytes)
std::string long_name(3000, 'a');
archive->append(long_name, data, size); // Will fail
```

### 7. Consider Memory Usage for Large Files

```cpp
// For small files - read all at once
auto data = archive->read("small.txt");

// For large files - consider streaming
// (Note: Current API reads entire entry into memory)
// Future enhancement could add streaming read/write
```

## Limitations

### Current Limitations

1. **No Compression**: Files are stored uncompressed
2. **No Encryption**: Data is stored in plain text
3. **No Metadata**: No timestamps, permissions, or attributes
4. **No Modification**: Can only append or remove from end
5. **Memory Loading**: Entire entry is read into memory at once
6. **Name Length**: Entry names limited to 2048 bytes
7. **No Directories**: Directory structure preserved only in names

### When to Use

**Good for:**
- Simple file bundling
- Fast random access to entries
- Cases where compression is not needed
- Embedding resources in applications
- Quick prototyping and testing

**Not ideal for:**
- Large file compression (use tar.gz, zip, etc.)
- Secure data storage (no encryption)
- Frequently modified archives (footer rewrite on each change)
- Very large individual entries (full read into memory)

## Comparison with Other Formats

| Feature         | .ywa | .zip | .tar | .tar.gz |
|-----------------|------|------|------|---------|
| Compression     | - | ✓ | - | ✓ |
| Random access   | ✓ | ✓ | - | - |
| Streaming       | - | ✓ | ✓ | ✓ |
| Integrity check | CRC32 | CRC32 | - | - |
| Metadata        | - | ✓ | ✓ | ✓ |
| Modification    | Append/remove | ✓ | Append | - |
| Complexity      | Very low | Medium | Low | Medium |
| Tool support    | ywlib only | Universal | Universal | Universal |

## See Also

- [yw/archive.h](../ywlib/yw/archive.h) - Source code with full implementation
- [yw/file.h](../ywlib/yw/file.h) - File I/O primitives used by archive
- [yw/core.h](../ywlib/yw/core.h) - Core utilities and error handling
- [file-operations.md](file-operations.md) - File operations documentation
- [error-handling.md](error-handling.md) - Error handling documentation
