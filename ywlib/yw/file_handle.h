#pragma once
#include <yw/file.h>
#include <yw/handle_base.h>

namespace yw::file {
enum class open_mode { unknown, read_existing, update_existing, create_always, create_new, append, update_or_create };
enum class seek_whence { begin = SEEK_SET, current = SEEK_CUR, end = SEEK_END };
} // namespace yw::file

#include <fcntl.h>

#ifdef _WIN32
#include <io.h>
namespace yw::file::internal {
inline std::expected<FILE*, error> _open(const wchar_t* p, open_mode m) {
  const auto generic_read_write = GENERIC_READ | GENERIC_WRITE;
  DWORD desired = 0, disp = 0, share = FILE_SHARE_READ;
  const char* fdopen_mode = nullptr;
  int osf_flags = _O_BINARY;
  switch (m) {
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
  default: return std::unexpected(error(errors::invalid_argument, "invalid file open mode"));
  }
  const auto h = ::CreateFileW(p, desired, share, nullptr, disp, FILE_ATTRIBUTE_NORMAL, nullptr);
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
  if (const auto pos = ::_ftelli64(f); pos < 0)
    return std::unexpected(error(errors::operation_failed, "failed to tell position for truncation", errno));
  else if (const auto ec = ::_chsize_s(::_fileno(f), pos); ec != 0)
    return std::unexpected(error(errors::operation_failed, "failed to truncate file", static_cast<int32_t>(ec)));
  return {};
}
} // namespace yw::file::internal
#else
#include <unistd.h>
namespace yw::file::internal {
inline std::expected<FILE*, error> _open(const char* p, open_mode m) {
  int flags = 0;
  const char* fdopen_mode = nullptr;
  switch (m) {
  case open_mode::read_existing: flags = O_RDONLY, fdopen_mode = "rb"; break;
  case open_mode::update_existing: flags = O_RDWR, fdopen_mode = "r+b"; break;
  case open_mode::create_always: flags = O_RDWR | O_CREAT | O_TRUNC, fdopen_mode = "r+b"; break;
  case open_mode::create_new: flags = O_RDWR | O_CREAT | O_EXCL, fdopen_mode = "r+b"; break;
  case open_mode::append: flags = O_WRONLY | O_CREAT | O_APPEND, fdopen_mode = "ab"; break;
  case open_mode::update_or_create: flags = O_RDWR | O_CREAT, fdopen_mode = "r+b"; break;
  default: return std::unexpected(error(errors::invalid_argument, "invalid file open mode"));
  }
  const mode_t perms = 0666;
  if (int fd = ::open(p, flags, perms); fd == -1) {
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
} // namespace yw::file::internal
#endif

namespace yw::file {

class handle : public handle_base {
public:
  struct slot : handle_base::slot {
    std::FILE* file = nullptr;
    yw::string<path_char> path;
    open_mode mode = open_mode::unknown;

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
      if (!file) return {};
      if (std::fclose(std::exchange(file, nullptr)) != 0)
        return std::unexpected(error(errors::operation_failed, "failed to close file", errno));
      else return {};
    }

    std::expected<int64_t, error> rest() {
      if (!file) return std::unexpected(error(errors::not_initialized));
      if (auto cur = tell(); !cur) return cur.error().relay();
      else if (auto res = seek(0, seek_whence::end); !res) return res.error().relay();
      else if (auto end = tell(); !end) return end.error().relay();
      else if (auto res = seek(*cur, seek_whence::begin); !res) return res.error().relay();
      else return static_cast<int64_t>(*end - *cur);
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
      if (const auto n = std::fwrite(src, 1, bytes, file); n != 0) return n;
      if (std::ferror(file)) return std::unexpected(error(errors::operation_failed, "write error", errno));
      else return 0;
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

  ~handle() noexcept { close(); }

  handle() noexcept = default;
  handle(handle&&) noexcept = default;
  handle& operator=(handle&&) noexcept = default;

  handle(stringable auto&& Path, open_mode m, const source_line& sl = here()) {
    if (auto res = create(static_cast<decltype(Path)&&>(Path), m)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<handle, error> create(stringable auto&& Path, open_mode m) {
    const auto sp = handle_base::make_slot<handle>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    auto p = unicode<path_char>(static_cast<decltype(Path)&&>(Path));
    if (auto res = internal::_open(p.c_str(), m); !res) {
      erase_slot(sp->id);
      return res.error().relay();
    } else sp->file = *res;
    sp->path = std::move(p), sp->mode = m;
    return std::move(make_handle<handle>(sp->id));
  }

  /// returns path string used when opening file.
  const yw::string<path_char>& path(this auto& self) {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off(); // fatal
    return sp->path;
  }

  open_mode mode() const {
    if (const auto sp = get_slot(this); !sp) {
      error(errors::invalid_slotid).fizzle_out(); // warning
      return open_mode::unknown;
    } else return sp->mode;
  }

  bool is_open() const noexcept {
    if (const auto sp = get_slot(this); !sp) {
      error(errors::invalid_slotid).fizzle_out(); // warning
      return false;
    } else return sp->file != nullptr;
  }

  explicit operator bool() const noexcept { return is_open(); }

  std::expected<void, error> close() {
    if (const auto sp = get_slot(this))
      if (auto res = sp->close(); !res) return res.error().relay();
    return {};
  }

  int64_t tell() const {
    if (const auto sp = get_slot(this)) {
      if (auto res = sp->tell()) return *res;
      else res.error().add_footprint().fizzle_out();
    } else error(errors::invalid_slotid).fizzle_out();
    return 0;
  }

  std::expected<void, error> seek(integral auto off, seek_whence w = seek_whence::begin) {
    if (const auto sp = get_slot(this); !sp) return std::unexpected(error(errors::invalid_slotid));
    else if (auto res = sp->seek(static_cast<int64_t>(off), w)) return {};
    else return res.error().relay();
  }

  int64_t rest() const {
    if (const auto sp = get_slot(this)) {
      if (auto res = sp->rest()) return *res;
      else res.error().add_footprint().fizzle_out();
    } else error(errors::invalid_slotid).fizzle_out();
    return 0;
  }

  std::expected<size_t, error> read(void* dst, size_t bytes) {
    if (const auto sp = get_slot(this); !sp) return std::unexpected(error(errors::invalid_slotid));
    else if (auto res = sp->read(dst, bytes)) return *res;
    else return res.error().relay();
  }

  std::expected<void, error> read_exact(void* dst, size_t bytes) {
    if (const auto sp = get_slot(this)) {
      for (size_t total = 0; total < bytes;) {
        if (auto res = sp->read(static_cast<std::byte*>(dst) + total, bytes - total); !res) return res.error().relay();
        else if (*res == 0) return std::unexpected(error(errors::operation_failed, "unexpected end of file"));
        else total += *res;
      }
      return {};
    } else return std::unexpected(error(errors::invalid_slotid));
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
    if (const auto sp = get_slot(this)) {
      if (auto res = sp->rest()) {
        string<char> result(yw::min(static_cast<size_t>(*res), Max));
        if (auto res = read_exact(result.data(), result.size())) return result;
        else res.error().add_footprint().fizzle_out();
      } else res.error().add_footprint().fizzle_out();
    }
    return {};
  }

  std::expected<size_t, error> write(const void* src, size_t bytes) {
    if (const auto sp = get_slot(this); !sp) return std::unexpected(error(errors::invalid_slotid));
    else if (auto res = sp->write(src, bytes)) return *res;
    else return res.error().relay();
  }

  template<contiguous_iterator It, sized_sentinel_for<It> Se> requires trivial<iter_value_t<It>>
  std::expected<size_t, error> write(It first, Se last) {
    if (const auto n = std::ranges::distance(first, last); n == 0) return 0;
    else if (n < 0) return std::unexpected(error(errors::invalid_argument, "invalid iterator range"));
    else return write(std::to_address(first), static_cast<size_t>(n) * sizeof(iter_value_t<It>));
  }

  template<contiguous_range R> requires trivial<iter_value_t<R>> std::expected<size_t, error> write(R&& range) {
    return write(std::ranges::data(range), std::ranges::size(range) * sizeof(iter_value_t<R>));
  }

  std::expected<void, error> write_exact(const void* src, size_t bytes) {
    if (const auto sp = get_slot(this)) {
      const auto p = static_cast<const std::byte*>(src);
      for (size_t total = 0; total < bytes;) {
        if (auto res = sp->write(p + total, bytes - total); !res) return res.error().relay();
        else if (*res == 0) return std::unexpected(error(errors::operation_failed, "incomplete write"));
        else total += *res;
      }
      return {};
    } else return std::unexpected(error(errors::invalid_slotid));
  }

  template<contiguous_iterator It, sized_sentinel_for<It> Se> requires trivial<iter_value_t<It>>
  std::expected<void, error> write_exact(It first, Se last) {
    if (first >= last) return std::unexpected(error(errors::invalid_argument, "invalid iterator range"));
    return write_exact(std::to_address(first), std::ranges::distance(first, last) * sizeof(iter_value_t<It>));
  }

  template<contiguous_range R> requires trivial<iter_value_t<R>> std::expected<void, error> write_exact(R&& range) {
    return write_exact(std::ranges::data(range), std::ranges::size(range) * sizeof(iter_value_t<R>));
  }

  template<trivial T> std::expected<void, error> write_trivial(const T& v) {
    if (auto res = write_exact(&v, sizeof(T))) return {};
    else return res.error().relay();
  }

  template<typename T> requires is_bounded_array<T> && same_as<iter_value_t<T>, char>
  std::expected<void, error> write_literal(const T& arr) {
    return write_exact(arr, (arraysize(arr) - 1) * sizeof(char));
  }

  std::expected<void, error> flush() {
    if (const auto sp = get_slot(this); !sp) return std::unexpected(error(errors::invalid_slotid));
    else if (auto res = sp->flush()) return {};
    else return res.error().relay();
  }

  std::expected<void, error> truncate_to_current() {
    if (const auto sp = get_slot(this); !sp) return std::unexpected(error(errors::invalid_slotid));
    else if (auto res = sp->truncate_to_current()) return {};
    else return res.error().relay();
  }

  std::expected<void, error> close_at_current() {
    if (auto res = truncate_to_current(); !res) return res.error().relay();
    if (auto res = close(); !res) return res.error().relay();
    return {};
  }
};

inline handle open(stringable auto&& Path, open_mode m, const source_line& sl = here()) {
  if (auto res = handle::create(static_cast<decltype(Path)&&>(Path), m); !res) {
    res.error().add_footprint().fizzle_out(sl);
    return {};
  } else return std::move(*res);
}
} // namespace yw::file
