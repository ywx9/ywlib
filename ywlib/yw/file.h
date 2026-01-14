#pragma once
#include "yw/core.h"
#include "yw/error.h"
#include "yw/format.h"

#include <expected>
#include <filesystem>

namespace yw {

enum class open_mode { read_existing, update_existing, create_always, create_new, append, update_or_create };
enum class seek_whence { begin = SEEK_SET, current = SEEK_CUR, end = SEEK_END };

} // namespace yw

#ifdef _WIN32
#include <fcntl.h>
#include <io.h> // _fileno, _chsize_s
#include <windows.h>
namespace yw::internal {
inline std::string win32_last_error_string() {
  const DWORD err = ::GetLastError();
  if (err == 0) return "no error";
  LPSTR buf = nullptr;
  const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
  const DWORD n = ::FormatMessageA(flags, nullptr, err, 0, reinterpret_cast<LPSTR>(&buf), 0, nullptr);
  std::string s = (n && buf) ? std::string(buf, buf + n) : "unknown error";
  if (buf) ::LocalFree(buf);
  while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ')) s.pop_back(); // trim trailing
  return s;
}
inline std::expected<std::FILE*, error> handle_to_fileptr(HANDLE h, const char* mode) {
  if (h == INVALID_HANDLE_VALUE) return std::unexpected(yw_make_error(errors::invalid_argument));
  if (const int fd = ::_open_osfhandle(reinterpret_cast<intptr_t>(h), _O_BINARY); fd == -1) {
    ::CloseHandle(h);
    print_with_location()("_open_osfhandle failed: {}", win32_last_error_string());
    return std::unexpected(yw_make_error(errors::operation_failed));
  } else if (std::FILE* f = ::_fdopen(fd, mode); !f) {
    ::_close(fd);
    return std::unexpected(yw_make_error(errors::operation_failed));
  } else return f;
}
inline std::expected<FILE*, error> _open_win(const std::filesystem::path& p, open_mode mode) {
  const auto generic_read_write = GENERIC_READ | GENERIC_WRITE;
  DWORD desired = 0, disp = 0, share = FILE_SHARE_READ;
  const char* fdopen_mode = nullptr;
  switch (mode) {
  case open_mode::read_existing: desired = GENERIC_READ, disp = OPEN_EXISTING, fdopen_mode = "rb"; break;
  case open_mode::update_existing: desired = generic_read_write, disp = OPEN_EXISTING, fdopen_mode = "r+b"; break;
  case open_mode::create_always: desired = generic_read_write, disp = CREATE_ALWAYS, fdopen_mode = "w+b"; break;
  case open_mode::create_new: desired = generic_read_write, disp = CREATE_NEW, fdopen_mode = "w+b"; break;
  case open_mode::append: desired = FILE_APPEND_DATA, disp = OPEN_ALWAYS, fdopen_mode = "a+b"; break;
  case open_mode::update_or_create: desired = generic_read_write, disp = OPEN_ALWAYS, fdopen_mode = "r+b"; break;
  default: return std::unexpected(yw_make_error(errors::invalid_argument));
  }
  auto h = ::CreateFileW(p.c_str(), desired, share, nullptr, disp, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (h == INVALID_HANDLE_VALUE) {
    print_with_location()("CreateFileW failed: {}", win32_last_error_string());
    return std::unexpected(yw_make_error(errors::operation_failed));
  } else if (auto fexp = handle_to_fileptr(h, fdopen_mode); !fexp) return std::unexpected(fexp.error());
  else return fexp.value();
}
} // namespace yw::internal
#else
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
namespace yw::internal {
inline std::string posix_errno_string() { return std::string(std::strerror(errno)); }
inline std::expected<std::FILE*, error> fd_to_fileptr(int fd, const char* mode) {
  if (std::FILE* f = ::fdopen(fd, mode); !f) {
    ::close(fd);
    print_with_location()("fdopen failed: {}", posix_errno_string());
    return std::unexpected(yw_make_error(errors::operation_failed));
  } else return f;
}
inline std::expected<FILE*, error> _open_posix(const std::filesystem::path& p, open_mode mode) {
  int flags = 0;
  const char* fdopen_mode = nullptr;
  switch (mode) {
  case open_mode::read_existing: flags = O_RDONLY, fdopen_mode = "rb"; break;
  case open_mode::update_existing: flags = O_RDWR, fdopen_mode = "r+b"; break;
  case open_mode::create_always: flags = O_RDWR | O_CREAT | O_TRUNC, fdopen_mode = "r+b"; break;
  case open_mode::create_new: flags = O_RDWR | O_CREAT | O_EXCL, fdopen_mode = "r+b"; break;
  case open_mode::append: flags = O_WRONLY | O_CREAT | O_APPEND, fdopen_mode = "ab"; break;
  case open_mode::update_or_create: flags = O_RDWR | O_CREAT, fdopen_mode = "r+b"; break;
  default: return std::unexpected(yw_make_error(errors::invalid_argument));
  }
  const mode_t perms = 0666;
  if (int fd = ::open(p.c_str(), flags, perms); fd == -1) {
    print_with_location()("open failed: {}", posix_errno_string());
    return std::unexpected(yw_make_error(errors::operation_failed));
  } else if (auto fexp = fd_to_fileptr(fd, fdopen_mode); !fexp)
    return std::unexpected(yw_make_error(errors::operation_failed));
  else return fexp.value();
}
} // namespace yw::internal
#endif

namespace yw {

class file_handler {
  std::FILE* _file = nullptr;
  std::filesystem::path _path;

  explicit file_handler(std::filesystem::path path, std::FILE* file) : _path(std::move(path)), _file(file) {}

  std::expected<void, error> _seek(int64_t off, seek_whence w) const {
    if (!_file) return std::unexpected(yw_make_error(errors::not_initialized));
#ifdef _WIN32
    if (::_fseeki64(_file, static_cast<__int64>(off), static_cast<int>(w)) != 0)
#else
    if (::fseeko(_file, static_cast<off_t>(off), static_cast<int>(w)) != 0)
#endif
      return std::unexpected(yw_make_error(errors::operation_failed));
    else return {};
  }

public:
  file_handler() noexcept = default;
  file_handler(const file_handler&) = delete;
  file_handler& operator=(const file_handler&) = delete;
  file_handler(file_handler&& other) noexcept
    : _file(std::exchange(other._file, nullptr)), _path(std::move(other._path)) {}
  file_handler& operator=(file_handler&& other) noexcept {
    if (this == &other) return *this;
    if (_file) std::fclose(_file);
    _file = std::exchange(other._file, nullptr);
    _path = std::move(other._path);
    return *this;
  }
  ~file_handler() {
    if (_file) std::fclose(std::exchange(_file, nullptr));
  }

  static std::expected<file_handler, error> create(const std::filesystem::path& path, open_mode mode) {
#ifdef _WIN32
    if (auto fexp = internal::_open_win(path, mode); !fexp)
#else
    if (auto fexp = internal::_open_posix(path, mode); !fexp)
#endif
      return std::unexpected(fexp.error());
    else return file_handler(path, fexp.value());
  }

  const std::filesystem::path& path() const noexcept { return _path; }
  bool is_open() const noexcept { return _file != nullptr; }

  std::expected<void, error> close() {
    if (_file != nullptr && std::fclose(std::exchange(_file, nullptr)) != 0)
      return std::unexpected(yw_make_error(errors::operation_failed));
    return {};
  }

  std::expected<int64_t, error> tell() const {
    if (!_file) return std::unexpected(yw_make_error(errors::not_initialized));
#ifdef _WIN32
    if (auto pos = ::_ftelli64(_file); pos < 0)
#else
    if (auto pos = ::ftello(_file); pos < 0)
#endif
      return std::unexpected(yw_make_error(errors::operation_failed));
    else return static_cast<int64_t>(pos);
  }

  std::expected<void, error> seek(int64_t off, seek_whence w) { return _seek(off, w); }

  std::expected<int64_t, error> file_size() const {
    if (!_file) return std::unexpected(yw_make_error(errors::not_initialized));
    const auto cur = tell();
    if (!_seek(0, seek_whence::end)) return std::unexpected(yw_make_error(errors::operation_failed));
    const auto size = tell();
    if (!_seek(cur.value_or(0), seek_whence::begin)) return std::unexpected(yw_make_error(errors::operation_failed));
    return size;
  }

  std::expected<size_t, error> read(void* dst, size_t bytes) {
    if (!_file) return std::unexpected(yw_make_error(errors::not_initialized));
    if (bytes == 0) return 0;
    if (!dst) return std::unexpected(yw_make_error(errors::invalid_argument));
    if (const auto n = std::fread(dst, 1, bytes, _file); n != 0) return n;
    else if (std::ferror(_file)) return std::unexpected(yw_make_error(errors::operation_failed));
    else return 0;
  }

  std::expected<void, error> read_exact(void* dst, size_t bytes) {
    if (!_file) return std::unexpected(yw_make_error(errors::not_initialized));
    if (bytes == 0) return {};
    if (!dst) return std::unexpected(yw_make_error(errors::invalid_argument));
    std::byte* p = static_cast<std::byte*>(dst);
    for (size_t total = 0; total < bytes;) {
      if (const size_t n = std::fread(p + total, 1, bytes - total, _file); n != 0) total += n;
      else if (std::ferror(_file)) return std::unexpected(yw_make_error(errors::operation_failed));
      else return std::unexpected(yw_make_error(errors::operation_failed));
    }
    return {};
  }

  template<trivial T> std::expected<T, error> read_trivial() {
    T v{};
    if (auto res = read_exact(&v, sizeof(T)); !res) return std::unexpected(res.error());
    else return v;
  }

  /// writes up to 'bytes' from 'src' to the file.
  /// \return on success, ther number of bytes written (>0).
  ///         if `bytes` is 0, returns 0.
  /// \note a return value less than `bytes` indicates a partial write.
  std::expected<size_t, error> write(const void* src, size_t bytes) {
    if (!_file) return std::unexpected(yw_make_error(errors::not_initialized));
    if (bytes == 0) return 0;
    if (!src) return std::unexpected(yw_make_error(errors::invalid_argument));
    if (auto n = std::fwrite(src, 1, bytes, _file); n != 0) return n;
    else return std::unexpected(yw_make_error(errors::operation_failed));
  }

  /// writes exactly `bytes` bytes from `src` to the file.
  /// \return returns normally on success.
  /// \note returns an error if fewer than `bytes` bytes could be written.
  std::expected<void, error> write_exact(const void* src, size_t bytes) {
    if (!_file) return std::unexpected(yw_make_error(errors::not_initialized));
    if (bytes == 0) return {};
    if (!src) return std::unexpected(yw_make_error(errors::invalid_argument));
    const std::byte* p = static_cast<const std::byte*>(src);
    for (size_t total = 0; total < bytes;) {
      if (const auto n = std::fwrite(p + total, 1, bytes - total, _file); n != 0) total += n;
      else return std::unexpected(yw_make_error(errors::operation_failed));
    }
    return {};
  }

  /// writes a trivially copyable value to the file.
  /// \return returns normally on success.
  template<trivial T> std::expected<void, error> write_trivial(const T& v) { return write_exact(&v, sizeof(T)); }

  std::expected<void, error> flush() {
    if (!_file) return std::unexpected(yw_make_error(errors::not_initialized));
    if (std::fflush(_file) != 0) return std::unexpected(yw_make_error(errors::operation_failed));
    return {};
  }

  std::expected<void, error> truncate_to_current() {
    if (!_file) return std::unexpected(yw_make_error(errors::not_initialized));
    if (std::fflush(_file) != 0) return std::unexpected(yw_make_error(errors::operation_failed));
    const auto cur = tell();
    if (!cur) return std::unexpected(yw_make_error(errors::operation_failed));
#ifdef _WIN32
    if (const int fd = ::_fileno(_file); fd < 0) return std::unexpected(yw_make_error(errors::operation_failed));
    else if (::_chsize_s(fd, static_cast<__int64>(cur.value())) != 0)
#else
    if (const int fd = ::fileno(_file); fd < 0) return std::unexpected(yw_make_error(errors::operation_failed));
    else if (::ftruncate(fd, static_cast<off_t>(cur.value())) != 0)
#endif
      return std::unexpected(yw_make_error(errors::operation_failed));
    return {};
  }

  std::expected<void, error> close_at_current() {
    if (auto res = truncate_to_current(); !res) return std::unexpected(res.error());
    if (auto res = close(); !res) return std::unexpected(res.error());
    return {};
  }
};

inline std::expected<file_handler, error> open(const std::filesystem::path& path, open_mode mode) {
  return file_handler::create(path, mode);
}
} // namespace yw
