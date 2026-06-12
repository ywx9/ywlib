#pragma once
#include "yw/core.h"
#include "yw/error.h"

#include <fcntl.h>

namespace yw {

enum class open_mode { unknown, read_existing, update_existing, create_always, create_new, append, update_or_create };
enum class seek_whence { begin = SEEK_SET, current = SEEK_CUR, end = SEEK_END };

namespace errors {
inline constexpr error::kind file_invalid_mode{"file_invalid_mode"};
inline constexpr error::kind file_invalid_buffer{"file_invalid_buffer"};
inline constexpr error::kind file_open_failed{"file_open_failed"};
} // namespace errors

class file_handle;

} // namespace yw

#ifdef _WIN32
#include <io.h>
namespace yw::internal {
inline std::expected<FILE*, error> _open(const std::filesystem::path& p, open_mode mode) {
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
  default: return std::unexpected(error(errors::file_invalid_mode));
  }
  const auto h = ::CreateFileW(p.c_str(), desired, share, nullptr, disp, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (h != INVALID_HANDLE_VALUE) {
    if (const int fd = ::_open_osfhandle(reinterpret_cast<intptr_t>(h), osf_flags); fd == -1) {
      ::CloseHandle(h);
      return std::unexpected(error(errors::operation_failed, "_open_osfhandle failed", errno));
    } else if (std::FILE* f = ::_fdopen(fd, fdopen_mode); !f) {
      ::_close(fd);
      return std::unexpected(error(errors::file_open_failed, "_fdopen failed", errno));
    } else return f;
  } else return std::unexpected(error(errors::file_open_failed, "CreateFileW failed", int32_t(::GetLastError())));
}
} // namespace yw::internal
#else
#include <unistd.h>
namespace yw::internal {
inline std::expected<FILE*, error> _open(const std::filesystem::path& p, open_mode mode) {
  int flags = 0;
  const char* fdopen_mode = nullptr;
  switch (mode) {
  case open_mode::read_existing: flags = O_RDONLY, fdopen_mode = "rb"; break;
  case open_mode::update_existing: flags = O_RDWR, fdopen_mode = "r+b"; break;
  case open_mode::create_always: flags = O_RDWR | O_CREAT | O_TRUNC, fdopen_mode = "r+b"; break;
  case open_mode::create_new: flags = O_RDWR | O_CREAT | O_EXCL, fdopen_mode = "r+b"; break;
  case open_mode::append: flags = O_WRONLY | O_CREAT | O_APPEND, fdopen_mode = "ab"; break;
  case open_mode::update_or_create: flags = O_RDWR | O_CREAT, fdopen_mode = "r+b"; break;
  default: return std::unexpected(error(errors::file_invalid_mode, "invalid file mode", 0));
  }
  const mode_t perms = 0666;
  if (int fd = ::open(p.c_str(), flags, perms); fd == -1) {
    return std::unexpected(error(errors::file_open_failed, "open failed", errno));
  } else if (std::FILE* f = ::fdopen(fd, fdopen_mode); !f) {
    ::close(fd);
    return std::unexpected(error(errors::file_open_failed, "fdopen failed", errno));
  } else return f;
}
} // namespace yw::internal
#endif

namespace yw {

class file_handle : general_handle {
public:
  struct slot : general_handle::slot {
    std::FILE* file = nullptr;
    std::filesystem::path path;
    open_mode mode = open_mode::unknown;

    std::expected<void, error> seek(int64_t off, seek_whence w) {
      if (!file) return std::unexpected(error(errors::not_initialized));
#ifdef _WIN32
      if (::_fseeki64(file, static_cast<__int64>(off), static_cast<int>(w)) != 0)
#else
      if (::fseeko(file, static_cast<off_t>(off), static_cast<int>(w)) != 0)
#endif
        return std::unexpected(error(errors::operation_failed, "failed to seek", errno));
      else return {};
    }

    std::expected<int64_t, error> tell() {
      if (!file) return std::unexpected(error(errors::not_initialized));
#ifdef _WIN32
      if (auto pos = ::_ftelli64(file); pos < 0)
#else
      if (auto pos = ::ftello(file); pos < 0)
#endif
        return std::unexpected(error(errors::operation_failed, "failed to tell position", errno));
      else return static_cast<int64_t>(pos);
    }
  };

  using general_handle::general_handle;
  using general_handle::operator bool;

  const std::filesystem::path& path() const {
    const auto sp = static_cast<slot*>(internal::general_slotset.get(_id));
    if (!sp) error(errors::invalid_slotid).print_as_fatal();
    return sp->path;
  }

  const open_mode& mode() const {
    const auto sp = static_cast<slot*>(internal::general_slotset.get(_id));
    if (!sp) error(errors::invalid_slotid).print_as_fatal();
    return sp->mode;
  }

  bool is_open() const noexcept {
    const auto sp = static_cast<slot*>(internal::general_slotset.get(_id));
    return sp && sp->file != nullptr;
  }

  explicit operator bool() const noexcept { return is_open(); }

  std::expected<void, error> close() {
    if (const auto sp = static_cast<slot*>(internal::general_slotset.get(_id)))
      if (sp->file != nullptr && std::fclose(std::exchange(sp->file, nullptr)) != 0) {
        auto e = error(errors::operation_failed, "failed to close file", errno);
        return e.relay(sp->source_line);
      }
    return {};
  }

  int64_t tell() const {
    const auto sp = static_cast<slot*>(internal::general_slotset.get(_id));
    if (!sp) error(errors::invalid_slotid).print_as_fatal();
    if (auto res = sp->tell(); !res) res.error().add_footprint(sp->source_line).print_as_fatal();
    else return *res;
  }

  std::expected<void, error> seek(integral auto off, seek_whence w = seek_whence::begin) {
    const auto sp = static_cast<slot*>(internal::general_slotset.get(_id));
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = sp->seek(static_cast<int64_t>(off), w)) return {};
    else return res.error().relay(sp->file->source_line);
  }

  int64_t file_size() const {
    const auto sp = static_cast<slot*>(internal::general_slotset.get(_id));
    if (!sp) error(errors::invalid_slotid).print_as_fatal();
    if (!sp->file) error(errors::not_initialized).add_footprint(sp->source_line).print_as_fatal();
    if (auto cur = sp->tell(); !cur) cur.error().add_footprint(sp->source_line).print_as_fatal();
    else if (auto res = sp->seek(0, seek_whence::end); !res) res.error().add_footprint(sp->source_line).print_as_fatal();
    else if (auto size = sp->tell(); !size) size.error().add_footprint(sp->source_line).print_as_fatal();
    else if (auto res = sp->seek(*cur, seek_whence::begin); !res) res.error().add_footprint(sp->source_line).print_as_fatal();
    else return *size;
  }

public:
  // ~file_handle() {
  //   if (_file) std::fclose(std::exchange(_file, nullptr));
  // }

  // file_handle(const std::filesystem::path& path, open_mode mode) {
  //   if (auto res = internal::_open(path, mode); !res) {
  //     res.error().print_as_warning(true);
  //     return;
  //   } else *this = file_handle(path, *res, mode);
  // }

  // const std::filesystem::path& path() const noexcept { return _path; }

  // open_mode mode() const noexcept { return _mode; }

  // bool is_open() const noexcept { return _file != nullptr; }
  // explicit operator bool() const noexcept { return is_open(); }

  // std::expected<void, error> close() {
  //   if (_file != nullptr && std::fclose(std::exchange(_file, nullptr)) != 0)
  //     return std::unexpected(error(errors::operation_failed, "failed to close file", errno));
  //   return {};
  // }

  // int64_t tell() const {
  // }

  std::expected<size_t, error> read(void* dst, size_t bytes) {
    if (!_file) return std::unexpected(error(errors::not_initialized));
    if (bytes == 0) return 0;
    if (!dst) return std::unexpected(error(errors::file_invalid_buffer, "null destination buffer"));
    if (const auto n = std::fread(dst, 1, bytes, _file); n != 0) return n;
    else if (std::ferror(_file)) return std::unexpected(error(errors::operation_failed, "read error"));
    else return 0;
  }

  std::expected<void, error> read_exact(void* dst, size_t bytes) {
    if (!_file) return std::unexpected(error(errors::not_initialized));
    if (bytes == 0) return {};
    if (!dst) return std::unexpected(error(errors::file_invalid_buffer, "null destination buffer"));
    std::byte* p = static_cast<std::byte*>(dst);
    for (size_t total = 0; total < bytes;) {
      if (const size_t n = std::fread(p + total, 1, bytes - total, _file); n != 0) total += n;
      else if (std::ferror(_file)) return std::unexpected(error(errors::operation_failed, "read error"));
      else return std::unexpected(error(errors::operation_failed, "read error"));
    }
    return {};
  }

  template<trivial T> std::expected<T, error> read_trivial() {
    make_footprint(yw_file);
    T v{};
    if (auto res = read_exact(&v, sizeof(T)); !res) return std::unexpected(error(res.error()));
    else return v;
  }

  template<trivial T> std::expected<void, error> read_trivial(T& v) {
    make_footprint(yw_file);
    if (auto res = read_exact(&v, sizeof(T)); !res) return unexpected_error(res.error());
    else return {};
  }

  /// writes up to 'bytes' from 'src' to the file.
  /// \return on success, ther number of bytes written (>0).
  ///         if `bytes` is 0, returns 0.
  /// \note a return value less than `bytes` indicates a partial write.
  std::expected<size_t, error> write(const void* src, size_t bytes) {
    make_footprint(yw_file);
    if (!_file) return std::unexpected(error(errors::not_initialized));
    if (bytes == 0) return 0;
    if (!src) return std::unexpected(error(errors::file_invalid_buffer, "file_handle: null source buffer"));
    if (auto n = std::fwrite(src, 1, bytes, _file); n != 0) return n;
    else return std::unexpected(error(errors::operation_failed, "file_handle: write error"));
  }

  /// writes the contiguous range contents as raw bytes.
  /// \return on success, the number of bytes written.
  template<contiguous_range R> requires trivial<iter_value_t<R>> std::expected<size_t, error> write(R&& range) {
    return write(std::ranges::data(range), std::ranges::size(range) * sizeof(iter_value_t<R>));
  }

  /// writes exactly `bytes` bytes from `src` to the file.
  /// \return returns normally on success.
  /// \note returns an error if fewer than `bytes` bytes could be written.
  std::expected<void, error> write_exact(const void* src, size_t bytes) {
    make_footprint(yw_file);
    if (!_file) return std::unexpected(error(errors::not_initialized));
    if (bytes == 0) return {};
    if (!src) return std::unexpected(error(errors::file_invalid_buffer, "file_handle: null source buffer"));
    const std::byte* p = static_cast<const std::byte*>(src);
    for (size_t total = 0; total < bytes;) {
      if (const auto n = std::fwrite(p + total, 1, bytes - total, _file); n != 0) total += n;
      else return std::unexpected(error(errors::operation_failed, "file_handle: write error"));
    }
    return {};
  }

  /// writes the entire contiguous range contents as raw bytes.
  /// \return returns normally on success.
  template<contiguous_range R> requires trivial<iter_value_t<R>> std::expected<void, error> write_exact(R&& range) {
    return write_exact(std::ranges::data(range), std::ranges::size(range) * sizeof(iter_value_t<R>));
  }

  /// writes a trivially copyable value to the file.
  /// \return returns normally on success.
  template<trivial T> std::expected<void, error> write_trivial(const T& v) {
    make_footprint(yw_file);
    if (auto res = write_exact(&v, sizeof(T)); !res) return std::unexpected(error(res.error()));
    else return {};
  }

  template<typename T> requires is_bounded_array<T> && same_as<iter_value_t<T>, char>
  std::expected<void, error> write_literal(const T& arr) {
    return write_exact(arr, (arraysize(arr) - 1) * sizeof(char));
  }

  std::expected<void, error> flush() {
    make_footprint(yw_file);
    if (!_file) return std::unexpected(error(errors::not_initialized));
    if (std::fflush(_file) != 0) return std::unexpected(error(errors::operation_failed, "file_handle: flush error"));
    return {};
  }

  std::expected<void, error> truncate_to_current() {
    make_footprint(yw_file);
    if (!_file) return std::unexpected(error(errors::not_initialized));
    if (std::fflush(_file) != 0) return std::unexpected(error(errors::operation_failed, "file_handle: flush error"));
    const auto cur = tell();
    if (!cur) return std::unexpected(error(errors::operation_failed, "file_handle: tell error"));
#ifdef _WIN32
    if (const int fd = ::_fileno(_file); fd < 0)
      return std::unexpected(error(errors::operation_failed, "file_handle: fileno error", errno));
    else if (::_chsize_s(fd, static_cast<__int64>(*cur)) != 0)
#else
    if (const int fd = ::fileno(_file); fd < 0)
      return std::unexpected(error(errors::operation_failed, "file_handle: fileno error", errno));
    else if (::ftruncate(fd, static_cast<off_t>(*cur)) != 0)
#endif
      return std::unexpected(error(errors::operation_failed, "file_handle: truncate error", errno));
    return {};
  }

  std::expected<void, error> close_at_current() {
    make_footprint(yw_file);
    if (auto res = truncate_to_current(); !res) return std::unexpected(error(res.error()));
    if (auto res = close(); !res) return std::unexpected(error(res.error()));
    return {};
  }
};

inline file_handle open(const std::filesystem::path& path, open_mode mode) {
  make_footprint(yw_file);
  if (auto res = file_handle::create(path, mode); !res) {
    print.err(res.error());
    return {};
  } else return std::move(*res);
}
} // namespace yw

#undef ywlib_header_name
