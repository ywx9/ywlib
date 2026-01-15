#pragma once
#include "yw/core.h"

#include <fcntl.h>

namespace yw {

enum class open_mode { unknown, read_existing, update_existing, create_always, create_new, append, update_or_create };
enum class seek_whence { begin = SEEK_SET, current = SEEK_CUR, end = SEEK_END };

} // namespace yw

#ifdef _WIN32
namespace yw::internal {
inline std::expected<FILE*, error_trace> _open_win(const std::filesystem::path& p, open_mode mode) {
  const auto generic_read_write = GENERIC_READ | GENERIC_WRITE;
  DWORD desired = 0, disp = 0, share = FILE_SHARE_READ;
  const char* fdopen_mode = nullptr;
  int osf_flags = _O_BINARY;
  switch (mode) {
  case open_mode::read_existing:
    desired = GENERIC_READ, disp = OPEN_EXISTING, fdopen_mode = "rb", osf_flags = _O_RDONLY;
    break;
  case open_mode::update_existing:
    desired = generic_read_write, disp = OPEN_EXISTING, fdopen_mode = "r+b", osf_flags = _O_RDWR;
    break;
  case open_mode::create_always:
    desired = generic_read_write, disp = CREATE_ALWAYS, fdopen_mode = "w+b", osf_flags = _O_RDWR;
    break;
  case open_mode::create_new:
    desired = generic_read_write, disp = CREATE_NEW, fdopen_mode = "w+b", osf_flags = _O_RDWR;
    break;
  case open_mode::append:
    desired = FILE_APPEND_DATA, disp = OPEN_ALWAYS, fdopen_mode = "ab", osf_flags = _O_WRONLY | _O_APPEND;
    break;
  case open_mode::update_or_create:
    desired = generic_read_write, disp = OPEN_ALWAYS, fdopen_mode = "r+b", osf_flags = _O_RDWR;
    break;
  default: return unexpected_error(errors::invalid_argument, "file_handle::create: invalid open_mode");
  }
  auto h = ::CreateFileW(p.c_str(), desired, share, nullptr, disp, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (h == INVALID_HANDLE_VALUE) {
    return unexpected_error(errors::operation_failed, "file_handle::create: CreateFileW failed", int(::GetLastError()));
  } else {
    if (const int fd = ::_open_osfhandle(reinterpret_cast<intptr_t>(h), osf_flags); fd == -1) {
      ::CloseHandle(h);
      return unexpected_error(errors::operation_failed, "file_handle::create: _open_osfhandle failed", errno);
    } else if (std::FILE* f = ::_fdopen(fd, fdopen_mode); !f) {
      ::_close(fd);
      return unexpected_error(errors::operation_failed, "file_handle::create: _fdopen failed", errno);
    } else return f;
  }
}
} // namespace yw::internal
#else
#include <unistd.h>
namespace yw::internal {
inline std::expected<FILE*, error_trace> _open_posix(const std::filesystem::path& p, open_mode mode) {
  int flags = 0;
  const char* fdopen_mode = nullptr;
  switch (mode) {
  case open_mode::read_existing: flags = O_RDONLY, fdopen_mode = "rb"; break;
  case open_mode::update_existing: flags = O_RDWR, fdopen_mode = "r+b"; break;
  case open_mode::create_always: flags = O_RDWR | O_CREAT | O_TRUNC, fdopen_mode = "r+b"; break;
  case open_mode::create_new: flags = O_RDWR | O_CREAT | O_EXCL, fdopen_mode = "r+b"; break;
  case open_mode::append: flags = O_WRONLY | O_CREAT | O_APPEND, fdopen_mode = "ab"; break;
  case open_mode::update_or_create: flags = O_RDWR | O_CREAT, fdopen_mode = "r+b"; break;
  default: return unexpected_error(errors::invalid_argument, "file_handle::create: invalid open_mode");
  }
  const mode_t perms = 0666;
  if (int fd = ::open(p.c_str(), flags, perms); fd == -1) {
    return unexpected_error(errors::operation_failed, "file_handle::create: open failed", errno);
  } else if (std::FILE* f = ::fdopen(fd, fdopen_mode); !f) {
    ::close(fd);
    return unexpected_error(errors::operation_failed, "file_handle::create: fdopen failed", errno);
  } else return f;
}
} // namespace yw::internal
#endif

namespace yw {

class file_handle {
  std::FILE* _file = nullptr;
  std::filesystem::path _path;
  open_mode _mode = open_mode::unknown;

  explicit file_handle(std::filesystem::path path, std::FILE* file, open_mode mode)
    : _path(std::move(path)), _file(file), _mode(mode) {}
  std::expected<void, error_trace> _seek(int64_t off, seek_whence w) const {
    if (!_file) return unexpected_error(errors::not_initialized, "file_handle: not initialized");
#ifdef _WIN32
    if (::_fseeki64(_file, static_cast<__int64>(off), static_cast<int>(w)) != 0)
#else
    if (::fseeko(_file, static_cast<off_t>(off), static_cast<int>(w)) != 0)
#endif
      return unexpected_error(errors::operation_failed, "file_handle: failed to seek", errno);
    else return {};
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

  static std::expected<file_handle, error_trace> create(const std::filesystem::path& path, open_mode mode) {
#ifdef _WIN32
    if (auto fexp = internal::_open_win(path, mode); !fexp)
#else
    if (auto fexp = internal::_open_posix(path, mode); !fexp)
#endif
      return unexpected_error(fexp.error());
    else return file_handle(path, fexp.value(), mode);
  }

  const std::filesystem::path& path() const noexcept { return _path; }
  open_mode mode() const noexcept { return _mode; }
  bool is_open() const noexcept { return _file != nullptr; }

  std::expected<void, error_trace> close() {
    if (_file != nullptr && std::fclose(std::exchange(_file, nullptr)) != 0)
      return unexpected_error(errors::operation_failed, "file_handle: failed to close file", errno);
    return {};
  }

  std::expected<int64_t, error_trace> tell() const {
    if (!_file) return unexpected_error(errors::not_initialized, "file_handle: not initialized");
#ifdef _WIN32
    if (auto pos = ::_ftelli64(_file); pos < 0)
#else
    if (auto pos = ::ftello(_file); pos < 0)
#endif
      return unexpected_error(errors::operation_failed, "file_handle: failed to tell position", errno);
    else return static_cast<int64_t>(pos);
  }

  std::expected<void, error_trace> seek(int64_t off, seek_whence w) { return _seek(off, w); }

  std::expected<int64_t, error_trace> file_size() const {
    if (!_file) return unexpected_error(errors::not_initialized, "file_handle: not initialized");
    const auto cur = tell();
    if (!_seek(0, seek_whence::end))
      return unexpected_error(errors::operation_failed, "file_handle: failed to seek to end");
    const auto size = tell();
    if (!_seek(cur.value_or(0), seek_whence::begin))
      return unexpected_error(errors::operation_failed, "file_handle: failed to seek to begin");
    return size;
  }

  std::expected<size_t, error_trace> read(void* dst, size_t bytes) {
    if (!_file) return unexpected_error(errors::not_initialized, "file_handle: not initialized");
    if (bytes == 0) return 0;
    if (!dst) return unexpected_error(errors::invalid_argument, "file_handle: null destination buffer");
    if (const auto n = std::fread(dst, 1, bytes, _file); n != 0) return n;
    else if (std::ferror(_file)) return unexpected_error(errors::operation_failed, "file_handle: read error");
    else return 0;
  }

  std::expected<void, error_trace> read_exact(void* dst, size_t bytes) {
    if (!_file) return unexpected_error(errors::not_initialized, "file_handle: not initialized");
    if (bytes == 0) return {};
    if (!dst) return unexpected_error(errors::invalid_argument, "file_handle: null destination buffer");
    std::byte* p = static_cast<std::byte*>(dst);
    for (size_t total = 0; total < bytes;) {
      if (const size_t n = std::fread(p + total, 1, bytes - total, _file); n != 0) total += n;
      else if (std::ferror(_file)) return unexpected_error(errors::operation_failed, "file_handle: read error");
      else return unexpected_error(errors::operation_failed, "file_handle: read error");
    }
    return {};
  }

  template<trivial T> std::expected<T, error_trace> read_trivial() {
    T v{};
    if (auto res = read_exact(&v, sizeof(T)); !res) return unexpected_error(res.error());
    else return v;
  }

  /// writes up to 'bytes' from 'src' to the file.
  /// \return on success, ther number of bytes written (>0).
  ///         if `bytes` is 0, returns 0.
  /// \note a return value less than `bytes` indicates a partial write.
  std::expected<size_t, error_trace> write(const void* src, size_t bytes) {
    if (!_file) return unexpected_error(errors::not_initialized, "file_handle: not initialized");
    if (bytes == 0) return 0;
    if (!src) return unexpected_error(errors::invalid_argument, "file_handle: null source buffer");
    if (auto n = std::fwrite(src, 1, bytes, _file); n != 0) return n;
    else return unexpected_error(errors::operation_failed, "file_handle: write error");
  }

  /// writes exactly `bytes` bytes from `src` to the file.
  /// \return returns normally on success.
  /// \note returns an error if fewer than `bytes` bytes could be written.
  std::expected<void, error_trace> write_exact(const void* src, size_t bytes) {
    if (!_file) return unexpected_error(errors::not_initialized, "file_handle: not initialized");
    if (bytes == 0) return {};
    if (!src) return unexpected_error(errors::invalid_argument, "file_handle: null source buffer");
    const std::byte* p = static_cast<const std::byte*>(src);
    for (size_t total = 0; total < bytes;) {
      if (const auto n = std::fwrite(p + total, 1, bytes - total, _file); n != 0) total += n;
      else return unexpected_error(errors::operation_failed, "file_handle: write error");
    }
    return {};
  }

  /// writes a trivially copyable value to the file.
  /// \return returns normally on success.
  template<trivial T> std::expected<void, error_trace> write_trivial(const T& v) {
    if (auto res = write_exact(&v, sizeof(T)); !res) return unexpected_error(res.error());
    else return {};
  }

  std::expected<void, error_trace> flush() {
    if (!_file) return unexpected_error(errors::not_initialized, "file_handle: not initialized");
    if (std::fflush(_file) != 0) return unexpected_error(errors::operation_failed, "file_handle: flush error", errno);
    return {};
  }

  std::expected<void, error_trace> truncate_to_current() {
    if (!_file) return unexpected_error(errors::not_initialized, "file_handle: not initialized");
    if (std::fflush(_file) != 0) return unexpected_error(errors::operation_failed, "file_handle: flush error", errno);
    const auto cur = tell();
    if (!cur) return unexpected_error(errors::operation_failed, "file_handle: tell error");
#ifdef _WIN32
    if (const int fd = ::_fileno(_file); fd < 0)
      return unexpected_error(errors::operation_failed, "file_handle: fileno error");
    else if (::_chsize_s(fd, static_cast<__int64>(cur.value())) != 0)
#else
    if (const int fd = ::fileno(_file); fd < 0)
      return unexpected_error(errors::operation_failed, "file_handle: fileno error");
    else if (::ftruncate(fd, static_cast<off_t>(cur.value())) != 0)
#endif
      return unexpected_error(errors::operation_failed, "file_handle: truncate error", errno);
    return {};
  }

  std::expected<void, error_trace> close_at_current() {
    if (auto res = truncate_to_current(); !res) return unexpected_error(res.error());
    if (auto res = close(); !res) return unexpected_error(res.error());
    return {};
  }
};

inline std::expected<file_handle, error_trace> open(const std::filesystem::path& path, open_mode mode) {
  if (auto res = file_handle::create(path, mode); !res) return unexpected_error(res.error());
  else return std::move(res.value());
}
} // namespace yw
