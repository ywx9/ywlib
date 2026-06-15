#pragma once
#include "yw/slotset.h"

#include <fcntl.h>

namespace yw {

enum class open_mode { unknown, read_existing, update_existing, create_always, create_new, append, update_or_create };
enum class seek_whence { begin = SEEK_SET, current = SEEK_CUR, end = SEEK_END };

namespace errors {
inline constexpr error::kind invalid_open_mode{"invalid open mode"};
inline constexpr error::kind invalid_file_format{"invalid file format"};
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
  default: return std::unexpected(error(errors::invalid_open_mode));
  }
  const auto h = ::CreateFileW(p.c_str(), desired, share, nullptr, disp, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (h != INVALID_HANDLE_VALUE) {
    if (const int fd = ::_open_osfhandle(reinterpret_cast<intptr_t>(h), osf_flags); fd == -1) {
      ::CloseHandle(h);
      return std::unexpected(error(errors::operation_failed, "_open_osfhandle failed", errno));
    } else if (std::FILE* f = ::_fdopen(fd, fdopen_mode); !f) {
      ::_close(fd);
      return std::unexpected(error(errors::operation_failed, "_fdopen failed", errno));
    } else return f;
  } else return std::unexpected(error(errors::operation_failed, "CreateFileW failed", int32_t(::GetLastError())));
}
inline std::expected<void, error> _seek(FILE* f, int64_t off, seek_whence w) {
  if (::_fseeki64(f, static_cast<__int64>(off), static_cast<int>(w)) != 0)
    return std::unexpected(error(errors::operation_failed, "failed to seek", errno));
  else return {};
}
inline std::expected<int64_t, error> _tell(FILE* f) {
  if (auto pos = ::_ftelli64(f); pos < 0)
    return std::unexpected(error(errors::operation_failed, "failed to tell position", errno));
  else return static_cast<int64_t>(pos);
}
inline std::expected<void, error> _truncate(FILE* f) {
  if (::_chsize(::_fileno(f), ::_ftelli64(f)) != 0)
    return std::unexpected(error(errors::operation_failed, "failed to truncate file", errno));
  else return {};
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
  default: return std::unexpected(error(errors::invalid_open_mode, "invalid file mode", 0));
  }
  const mode_t perms = 0666;
  if (int fd = ::open(p.c_str(), flags, perms); fd == -1) {
    return std::unexpected(error(errors::operation_failed, "open failed", errno));
  } else if (std::FILE* f = ::fdopen(fd, fdopen_mode); !f) {
    ::close(fd);
    return std::unexpected(error(errors::operation_failed, "fdopen failed", errno));
  } else return f;
}
inline std::expected<void, error> _seek(FILE* f, int64_t off, seek_whence w) {
  if (::fseeko(f, static_cast<off_t>(off), static_cast<int>(w)) != 0)
    return std::unexpected(error(errors::operation_failed, "failed to seek", errno));
  else return {};
}
inline std::expected<int64_t, error> _tell(FILE* f) {
  if (auto pos = ::ftello(f); pos < 0)
    return std::unexpected(error(errors::operation_failed, "failed to tell position", errno));
  else return static_cast<int64_t>(pos);
}
inline std::expected<void, error> _truncate(FILE* f) {
  if (fflush(f) != 0) return std::unexpected(error(errors::operation_failed, "failed to flush file", errno));
  if (auto pos = ::ftello(f); pos < 0)
    return std::unexpected(error(errors::operation_failed, "failed to tell position for truncation", errno));
  else if (::ftruncate(fileno(f), static_cast<off_t>(pos)) != 0)
    return std::unexpected(error(errors::operation_failed, "failed to truncate file", errno));
  else return {};
}
} // namespace yw::internal
#endif

namespace yw {

/// MARK: file_handle

class file_handle : public general_handle {
public:
  struct slot : general_handle::slot {
    std::FILE* file = nullptr;
    std::filesystem::path path;
    open_mode mode = open_mode::unknown;

    std::expected<void, error> open() {
      if (file) return std::unexpected(error(errors::already_initialized));
      if (auto res = internal::_open(path, mode)) file = *res;
      else return res.error().relay();
      return {};
    }

    std::expected<void, error> seek(int64_t off, seek_whence w) {
      if (!file) return std::unexpected(error(errors::not_initialized));
      if (auto res = internal::_seek(file, off, w)) return {};
      else return res.error().relay();
    }

    std::expected<int64_t, error> tell() {
      if (!file) return std::unexpected(error(errors::not_initialized));
      if (auto res = internal::_tell(file)) return *res;
      else return res.error().relay();
    }

    std::expected<void, error> close() {
      if (!file) return std::unexpected(error(errors::not_initialized));
      if (std::fclose(std::exchange(file, nullptr)) != 0)
        return std::unexpected(error(errors::operation_failed, "failed to close file", errno));
      else return {};
    }

    std::expected<size_t, error> size() {
      if (!file) return std::unexpected(error(errors::not_initialized));
      if (auto cur = tell(); !cur) return cur.error().relay();
      else if (auto res = seek(0, seek_whence::end); !res) return res.error().relay();
      else if (auto size = tell(); !size) return size.error().relay();
      else if (auto res = seek(*cur, seek_whence::begin); !res) return res.error().relay();
      else return static_cast<size_t>(*size);
    }

    std::expected<size_t, error> read(void* dst, size_t bytes) {
      if (!file) return std::unexpected(error(errors::not_initialized));
      if (bytes == 0) return 0;
      if (!dst) return std::unexpected(error(errors::invalid_argument, "null destination buffer"));
      if (const auto n = std::fread(dst, 1, bytes, file); n != 0) return n;
      else if (std::ferror(file)) return std::unexpected(error(errors::operation_failed, "read error", errno));
      else return 0;
    }

    std::expected<size_t, error> write(const void* src, size_t bytes) {
      if (!file) return std::unexpected(error(errors::not_initialized));
      if (bytes == 0) return 0;
      if (!src) return std::unexpected(error(errors::invalid_argument, "null source buffer"));
      if (const auto n = std::fwrite(src, 1, bytes, file); n != bytes)
        return std::unexpected(error(errors::operation_failed, "write error", errno));
      else return n;
    }

    std::expected<void, error> flush() {
      if (!file) return std::unexpected(error(errors::not_initialized));
      if (std::fflush(file) != 0) return std::unexpected(error(errors::operation_failed, "flush error", errno));
      else return {};
    }

    std::expected<void, error> truncate_to_current() {
      if (!file) return std::unexpected(error(errors::not_initialized));
      if (auto res = internal::_truncate(file)) return {};
      else return res.error().relay();
    }
  };

  using general_handle::general_handle;

  file_handle(std::filesystem::path path, open_mode mode, const source_line& sl = source_line::here()) {
    const auto sp = create_slot<file_handle>(sl);
    if (!sp) error(errors::slot_creation_failed).print_as_fatal(sl);
    sp->path = std::move(path);
    sp->mode = mode;
    if (auto res = sp->open()) _id = sp->id;
    else res.error().print_as_warning(sl); // prints warning and leaves handle empty.
  }

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
      if (auto res = sp->close(); !res) return res.error().relay();
    return {};
  }

  int64_t tell() const {
    if (const auto sp = static_cast<slot*>(internal::general_slotset.get(_id))) {
      if (auto res = sp->tell()) return *res;
      else res.error().add_footprint().add_footprint(sp->source_line).print_as_warning();
    } else error(errors::invalid_slotid).print_as_warning();
    return 0;
  }

  std::expected<void, error> seek(integral auto off, seek_whence w = seek_whence::begin) {
    const auto sp = static_cast<slot*>(internal::general_slotset.get(_id));
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = sp->seek(static_cast<int64_t>(off), w)) return {};
    else return res.error().relay();
  }

  int64_t file_size() const {
    if (const auto sp = static_cast<slot*>(internal::general_slotset.get(_id))) {
      if (auto res = sp->size()) return *res;
      else res.error().add_footprint().add_footprint(sp->source_line).print_as_warning();
    } else error(errors::invalid_slotid).print_as_warning();
    return 0;
  }

  std::expected<size_t, error> read(void* dst, size_t bytes) {
    const auto sp = static_cast<slot*>(internal::general_slotset.get(_id));
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = sp->read(dst, bytes)) return *res;
    else return res.error().relay();
  }

  std::expected<void, error> read_exact(void* dst, size_t bytes) {
    const auto sp = static_cast<slot*>(internal::general_slotset.get(_id));
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    for (size_t total = 0; total < bytes;) {
      if (auto res = sp->read(static_cast<std::byte*>(dst) + total, bytes - total); !res) return res.error().relay();
      else if (*res == 0) return std::unexpected(error(errors::operation_failed, "unexpected end of file"));
      else total += *res;
    }
    return {};
  }

  template<trivial T> std::expected<T, error> read_trivial() {
    T v{};
    if (auto res = read_exact(&v, sizeof(T))) return v;
    else return res.error().relay();
  }

  template<trivial T> std::expected<void, error> read_trivial(T& v) {
    if (auto res = read_exact(&v, sizeof(T))) return {};
    else return res.error().relay();
  }

  string<char> read_as_string(size_t Max = npos) {
    if (const auto sp = static_cast<slot*>(internal::general_slotset.get(_id))) {
      if (auto res = sp->size()) {
        string<char> result(yw::min(static_cast<size_t>(*res), Max));
        if (auto res = read_exact(result.data(), result.capacity())) return result;
        else res.error().add_footprint().add_footprint(sp->source_line).print_as_warning();
      } else res.error().add_footprint().add_footprint(sp->source_line).print_as_warning();
    }
    return {};
  }

  std::expected<size_t, error> write(const void* src, size_t bytes) {
    const auto sp = static_cast<slot*>(internal::general_slotset.get(_id));
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = sp->write(src, bytes)) return *res;
    else return res.error().relay();
  }

  template<contiguous_iterator It, sentinel_for<It> Se> requires trivial<iter_value_t<It>>
  std::expected<size_t, error> write(It first, Se last) {
    return write(std::to_address(first), std::ranges::distance(first, last) * sizeof(iter_value_t<It>));
  }

  template<contiguous_range R> requires trivial<iter_value_t<R>> std::expected<size_t, error> write(R&& range) {
    return write(std::ranges::data(range), std::ranges::size(range) * sizeof(iter_value_t<R>));
  }

  std::expected<void, error> write_exact(const void* src, size_t bytes) {
    const auto sp = static_cast<slot*>(internal::general_slotset.get(_id));
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    const auto p = static_cast<const std::byte*>(src);
    for (size_t total = 0; total < bytes;) {
      if (auto res = sp->write(p + total, bytes - total); !res) return res.error().relay();
      else if (*res == 0) return std::unexpected(error(errors::operation_failed, "incomplete write"));
      else total += *res;
    }
    return {};
  }

  template<contiguous_iterator It, sentinel_for<It> Se> requires trivial<iter_value_t<It>>
  std::expected<void, error> write_exact(It first, Se last) {
    return write_exact(std::to_address(first), std::ranges::distance(first, last) * sizeof(iter_value_t<It>));
  }

  template<contiguous_range R> requires trivial<iter_value_t<R>> std::expected<void, error> write_exact(R&& range) {
    return write_exact(std::ranges::data(range), std::ranges::size(range) * sizeof(iter_value_t<R>));
  }

  template<trivial T> std::expected<void, error> write_trivial(const T& v) {
    if (auto res = write_exact(&v, sizeof(T))) return {};
    else return std::unexpected(error(res.error()));
  }

  template<typename T> requires is_bounded_array<T> && same_as<iter_value_t<T>, char>
  std::expected<void, error> write_literal(const T& arr) {
    return write_exact(arr, (arraysize(arr) - 1) * sizeof(char));
  }

  std::expected<void, error> flush() {
    const auto sp = static_cast<slot*>(internal::general_slotset.get(_id));
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = sp->flush()) return {};
    else return res.error().relay();
  }

  std::expected<void, error> truncate_to_current() {
    const auto sp = static_cast<slot*>(internal::general_slotset.get(_id));
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = sp->truncate_to_current()) return {};
    else return res.error().relay();
  }

  std::expected<void, error> close_at_current() {
    if (auto res = truncate_to_current(); !res) return res.error().relay();
    if (auto res = close(); !res) return res.error().relay();
    return {};
  }
};

inline file_handle open(const std::filesystem::path& path, open_mode mode, const source_line& sl = source_line::here()) {
  return file_handle(path, mode, sl);
}
} // namespace yw

#undef ywlib_header_name
