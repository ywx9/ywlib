#pragma once
#include "yw/core.h"

#include <filesystem>

#ifdef _WIN32
#include <io.h> // _fileno, _chsize_s
#else
#include <unistd.h> // fileno, ftruncate
#endif

namespace yw {

enum class open_mode {
  read_existing,
  update_existing,
  create_always,
  create_new,
  append,
  update_or_create,
};

enum class seek_whence { begin = SEEK_SET, current = SEEK_CUR, end = SEEK_END };

class file_handle;
inline std::expected<file_handle, std::string> open(const std::filesystem::path& path, open_mode mode);

class file_handle {
  friend std::expected<file_handle, std::string> open(const std::filesystem::path& path, open_mode mode);
  std::FILE* _file = nullptr;
  std::filesystem::path _path;

  /// \note only used by `open`
  explicit file_handle(std::filesystem::path path, std::FILE* file) : _path(std::move(path)), _file(file) {}

  bool _seek(int64_t off, seek_whence w) const {
    if (!_file) return false;
#ifdef _WIN32
    return ::_fseeki64(_file, static_cast<__int64>(off), static_cast<int>(w)) == 0;
#else
    return ::fseeko(_file, static_cast<off_t>(off), static_cast<int>(w)) == 0;
#endif
  }

public:
  file_handle() noexcept = default;
  file_handle(const file_handle&) = delete;
  file_handle& operator=(const file_handle&) = delete;
  file_handle(file_handle&& other) noexcept
    : _file(std::exchange(other._file, nullptr)), _path(std::move(other._path)) {}
  file_handle& operator=(file_handle&& other) noexcept {
    if (this == &other) return *this;
    if (_file) std::fclose(_file);
    _file = std::exchange(other._file, nullptr);
    _path = std::move(other._path);
    return *this;
  }
  ~file_handle() {
    if (_file) std::fclose(std::exchange(_file, nullptr));
  }

  const std::filesystem::path& path() const noexcept { return _path; }
  bool is_open() const noexcept { return _file != nullptr; }

  void close() {
    if (_file) std::fclose(std::exchange(_file, nullptr));
  }

  std::optional<int64_t> tell() const {
#ifdef _WIN32
    return ::_ftelli64(_file);
#else
    return ::ftello(_file);
#endif
  }

  bool seek(int64_t off, seek_whence w) { return _seek(off, w); }

  std::optional<int64_t> file_size() const {
    if (!_file) return std::nullopt;
    const auto cur = tell();
    if (!_seek(0, seek_whence::end)) return std::nullopt;
    const auto size = tell();
    if (!_seek(cur.value_or(0), seek_whence::begin)) return std::nullopt;
    return size;
  }

  std::expected<size_t, std::string> read(void* dst, size_t bytes) {
    if (!_file) return std::unexpected("file not open");
    if (bytes == 0) return 0;
    if (!dst) return std::unexpected("null destination pointer");
    if (const auto n = std::fread(dst, 1, bytes, _file); n != 0) return n;
    else if (std::ferror(_file)) return std::unexpected("read error");
    else return 0; // EOF
  }

  std::expected<void, std::string> read_exact(void* dst, size_t bytes) {
    if (!_file) return std::unexpected("file not open");
    if (bytes == 0) return {};
    if (!dst) return std::unexpected("null destination pointer");
    std::byte* p = static_cast<std::byte*>(dst);
    for (size_t total = 0; total < bytes;) {
      if (const size_t n = std::fread(p + total, 1, bytes - total, _file); n != 0) total += n;
      else if (std::ferror(_file)) return std::unexpected("read error");
      else return std::unexpected("unexpected end of file");
    }
    return {};
  }

  template<trivial T> std::expected<T, std::string> read_trivial() {
    T v{};
    if (auto res = read_exact(&v, sizeof(T)); !res) return std::unexpected(res.error());
    else return v;
  }

  template<trivial T> std::expected<std::string, std::string> read_as_string(size_t bytes = npos) {
    if (!_file) return std::unexpected("file not open");
    if (bytes == npos) {
      auto size_opt = file_size();
      if (!size_opt) return std::unexpected("failed to get file size");
      const auto size = size_opt.value();
      const auto cur = tell();
      if (!cur) return std::unexpected("failed to get current position");
      if (size < *cur) return std::unexpected("invalid file size");
      bytes = static_cast<size_t>(size - *cur);
    }
    std::string str(bytes, '\0');
    if (auto res = read_exact(str.data(), bytes); !res) return std::unexpected(res.error());
    else return str;
  }

  /// writes up to 'bytes' from 'src' to the file.
  /// \return on success, ther number of bytes written (>0).
  ///         if `bytes` is 0, returns 0.
  /// \note a return value less than `bytes` indicates a partial write.
  std::expected<size_t, std::string> write(const void* src, size_t bytes) {
    if (!_file) return std::unexpected("file not open");
    if (bytes == 0) return 0;
    if (!src) return std::unexpected("null source pointer");
    if (auto n = std::fwrite(src, 1, bytes, _file); n != 0) return n;
    else if (std::ferror(_file)) return std::unexpected("write error");
    else return std::unexpected("write made no progress");
  }

  /// writes exactly `bytes` bytes from `src` to the file.
  /// \return returns normally on success.
  /// \note returns an error if fewer than `bytes` bytes could be written.
  std::expected<void, std::string> write_exact(const void* src, size_t bytes) {
    if (!_file) return std::unexpected("file not open");
    if (bytes == 0) return {};
    if (!src) return std::unexpected("null source pointer");
    const std::byte* p = static_cast<const std::byte*>(src);
    for (size_t total = 0; total < bytes;) {
      if (const auto n = std::fwrite(p + total, 1, bytes - total, _file); n != 0) total += n;
      else if (std::ferror(_file)) return std::unexpected("write error");
      else return std::unexpected("write made no progress");
    }
    return {};
  }

  /// writes a trivially copyable value to the file.
  /// \return returns normally on success.
  template<trivial T> std::expected<void, std::string> write_trivial(const T& v) { return write_exact(&v, sizeof(T)); }

  std::expected<void, std::string> flush() {
    if (!_file) return std::unexpected("file not open");
    if (std::fflush(_file) != 0) return std::unexpected("flush failed");
    return {};
  }

  std::expected<void, std::string> truncate_to_current() {
    if (!_file) return std::unexpected("file not open");
    if (std::fflush(_file) != 0) return std::unexpected("flush failed");
    const auto cur = tell();
    if (!cur) return std::unexpected("invalid tell position");
#ifdef _WIN32
    if (const int fd = ::_fileno(_file); fd < 0) return std::unexpected("fileno failed");
    else if (::_chsize_s(fd, static_cast<__int64>(cur.value())) != 0) return std::unexpected("truncate failed");
#else
    if (const int fd = ::fileno(_file); fd < 0) return std::unexpected("fileno failed");
    else if (::ftruncate(fd, static_cast<off_t>(cur.value())) != 0) return std::unexpected("truncate failed");
#endif
    return {};
  }

  void close_at_current() {
    truncate_to_current();
    close();
  }
};

namespace internal {

std::expected<file_handle, std::string> _open_win(const std::filesystem::path& path, open_mode mode) {

}
}
} // namespace yw
