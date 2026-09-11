#pragma once
#include <yw/file.h>
#include <yw/property.h>

namespace yw::file {
enum class open_mode { unknown, read_existing, update_existing, create_always, create_new, append, update_or_create };
enum class seek_whence { begin = SEEK_SET, current = SEEK_CUR, end = SEEK_END };
} // namespace yw::file

#include <fcntl.h>

#ifdef _WIN32
#include <io.h>
namespace yw::file::internal {
inline result<FILE*> _open(const wchar_t* p, open_mode m) {
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
  default: return fail<FILE*>(errors::invalid_argument, "invalid file open mode");
  }
  const auto h = ::CreateFileW(p, desired, share, nullptr, disp, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (h != INVALID_HANDLE_VALUE) {
    if (const int fd = ::_open_osfhandle(reinterpret_cast<intptr_t>(h), osf_flags); fd == -1) {
      ::CloseHandle(h);
      return fail<FILE*>(errors::operation_failed, "_open_osfhandle failed", errno);
    } else if (std::FILE* f = ::_fdopen(fd, fdopen_mode); !f) {
      ::_close(fd);
      return fail<FILE*>(errors::operation_failed, "_fdopen failed", errno);
    } else return f;
  } else
    return fail<FILE*>(
      errors::operation_failed, format("CreateFileW failed: ", *p ? string_view<wchar_t>(p) : L"<empty>"sv),
      int32_t(::GetLastError()));
}
inline result<void> _seek(FILE* f, int64_t off, seek_whence w) {
  if (::_fseeki64(f, static_cast<__int64>(off), static_cast<int>(w)) != 0)
    return fail<void>(errors::operation_failed, "failed to seek", errno);
  else return {};
}
inline result<int64_t> _tell(FILE* f) {
  if (auto pos = ::_ftelli64(f); pos < 0)
    return fail<int64_t>(errors::operation_failed, "failed to tell position", errno);
  else return static_cast<int64_t>(pos);
}
inline result<void> _truncate(FILE* f) {
  if (const auto pos = ::_ftelli64(f); pos < 0)
    return fail<void>(errors::operation_failed, "failed to tell position for truncation", errno);
  else if (const auto ec = ::_chsize_s(::_fileno(f), pos); ec != 0)
    return fail<void>(errors::operation_failed, "failed to truncate file", static_cast<int32_t>(ec));
  return {};
}
} // namespace yw::file::internal
#else
#include <unistd.h>
namespace yw::file::internal {
inline result<FILE*> _open(const char* p, open_mode m) {
  int flags = 0;
  const char* fdopen_mode = nullptr;
  switch (m) {
  case open_mode::read_existing: flags = O_RDONLY, fdopen_mode = "rb"; break;
  case open_mode::update_existing: flags = O_RDWR, fdopen_mode = "r+b"; break;
  case open_mode::create_always: flags = O_RDWR | O_CREAT | O_TRUNC, fdopen_mode = "r+b"; break;
  case open_mode::create_new: flags = O_RDWR | O_CREAT | O_EXCL, fdopen_mode = "r+b"; break;
  case open_mode::append: flags = O_WRONLY | O_CREAT | O_APPEND, fdopen_mode = "ab"; break;
  case open_mode::update_or_create: flags = O_RDWR | O_CREAT, fdopen_mode = "r+b"; break;
  default: return fail<FILE*>(errors::invalid_argument, "invalid file open mode");
  }
  const mode_t perms = 0666;
  if (int fd = ::open(p, flags, perms); fd == -1) {
    return fail<FILE*>(
      errors::operation_failed, format("open failed: ", *p ? string_view<char>(p) : "<empty>"sv), errno);
  } else if (std::FILE* f = ::fdopen(fd, fdopen_mode); !f) {
    ::close(fd);
    return fail<FILE*>(errors::operation_failed, "fdopen failed", errno);
  } else return f;
}
inline result<void> _seek(FILE* f, int64_t off, seek_whence w) {
  if (::fseeko(f, static_cast<off_t>(off), static_cast<int>(w)) != 0)
    return fail<void>(errors::operation_failed, "failed to seek", errno);
  else return {};
}
inline result<int64_t> _tell(FILE* f) {
  if (auto pos = ::ftello(f); pos < 0)
    return fail<int64_t>(errors::operation_failed, "failed to tell position", errno);
  else return static_cast<int64_t>(pos);
}
inline result<void> _truncate(FILE* f) {
  if (fflush(f) != 0) return fail<void>(errors::operation_failed, "failed to flush file", errno);
  if (auto pos = ::ftello(f); pos < 0)
    return fail<void>(errors::operation_failed, "failed to tell position for truncation", errno);
  else if (::ftruncate(fileno(f), static_cast<off_t>(pos)) != 0)
    return fail<void>(errors::operation_failed, "failed to truncate file", errno);
  else return {};
}
} // namespace yw::file::internal
#endif

namespace yw::file {

class handle {
  std::FILE* _file = nullptr;

public:
  const_property<path_string, handle> path;
  const_property<open_mode, handle> mode = open_mode::unknown;

  ~handle() noexcept { close(); }

  handle() noexcept = default;
  handle(const handle&) = delete;
  handle& operator=(const handle&) = delete;

  handle(handle&& Other) noexcept
    : _file(exchange(Other._file, nullptr)), path(move(Other.path())), mode(Other.mode()) {
    Other.path = path_string{};
    Other.mode = open_mode::unknown;
  }

  handle& operator=(handle&& Other) noexcept {
    if (this == &Other) return *this;
    close();
    _file = exchange(Other._file, nullptr);
    path = move(Other.path());
    mode = Other.mode();
    Other.path = path_string{};
    Other.mode = open_mode::unknown;
    return *this;
  }

  handle(stringable auto&& Path, open_mode m, const source_line& sl = here()) {
    if (auto res = create(static_cast<decltype(Path)&&>(Path), m)) *this = move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static result<handle> create(stringable auto&& Path, open_mode m) {
    auto p = unicode<path_char>(static_cast<decltype(Path)&&>(Path));
    auto f = internal::_open(p.c_str(), m);
    if (!f) return f.relay();
    handle h;
    h._file = *f;
    h.path = move(p);
    h.mode = m;
    return h;
  }

  bool is_open() const noexcept { return _file != nullptr; }

  explicit operator bool() const noexcept { return is_open(); }

  result<void> close() {
    if (!_file) return {};
    if (std::fclose(exchange(_file, nullptr)) != 0)
      return fail<void>(errors::operation_failed, "failed to close file", errno);
    return {};
  }

  int64_t tell() const {
    if (!_file) {
      error(errors::not_initialized).fizzle_out();
      return 0;
    }
    if (auto res = internal::_tell(_file)) return *res;
    else res.error().add_footprint().fizzle_out();
    return 0;
  }

  result<void> seek(integral auto off, seek_whence w = seek_whence::begin) {
    if (!_file) return std::unexpected(error(errors::not_initialized));
    if (auto res = internal::_seek(_file, static_cast<int64_t>(off), w)) return {};
    else return res.relay();
  }

  int64_t rest() const {
    if (!_file) {
      error(errors::not_initialized).fizzle_out();
      return 0;
    }
    const auto self = const_cast<handle*>(this);
    if (auto cur = internal::_tell(_file); !cur) cur.error().add_footprint().fizzle_out();
    else if (auto res = self->seek(0, seek_whence::end); !res) res.error().add_footprint().fizzle_out();
    else if (auto end = internal::_tell(_file); !end) end.error().add_footprint().fizzle_out();
    else if (auto res = self->seek(*cur, seek_whence::begin); !res) res.error().add_footprint().fizzle_out();
    else return static_cast<int64_t>(*end - *cur);
    return 0;
  }

  result<size_t> read(void* dst, size_t bytes) {
    if (!_file) return fail<size_t>(errors::not_initialized);
    if (bytes == 0) return 0;
    if (!dst) return fail<size_t>(errors::invalid_argument, "null destination buffer");
    if (const auto n = std::fread(dst, 1, bytes, _file); n != 0) return n;
    if (std::ferror(_file)) return fail<size_t>(errors::operation_failed, "read error", errno);
    return 0;
  }

  result<void> read_exact(void* dst, size_t bytes) {
    for (size_t total = 0; total < bytes;) {
      if (auto res = read(static_cast<std::byte*>(dst) + total, bytes - total); !res) return res.relay_error<void>();
      else if (*res == 0) return fail<void>(errors::operation_failed, "unexpected end of file");
      else total += *res;
    }
    return {};
  }

  template<trivial T> result<T> read_trivial() {
    T v{};
    if (auto res = read_exact(&v, sizeof(T))) return v;
    else return res.relay();
  }

  template<trivial T> result<void> read_trivial(T& v) {
    if (auto res = read_exact(&v, sizeof(T))) return {};
    else return res.relay();
  }

  string<char> read_as_string(size_t Max = npos) {
    if (const auto remaining = rest(); remaining > 0) {
      string<char> result(yw::min(static_cast<size_t>(remaining), Max));
      if (auto res = read_exact(result.data(), result.size())) return result;
      else res.error().add_footprint().fizzle_out();
    }
    return {};
  }

  result<size_t> write(const void* src, size_t bytes) {
    if (!_file) return fail<size_t>(errors::not_initialized);
    if (bytes == 0) return 0;
    if (!src) return fail<size_t>(errors::invalid_argument, "null source buffer");
    if (const auto n = std::fwrite(src, 1, bytes, _file); n != 0) return n;
    if (std::ferror(_file)) return fail<size_t>(errors::operation_failed, "write error", errno);
    return 0;
  }

  template<contiguous_iterator It, sized_sentinel_for<It> Se> requires trivial<iter_value_t<It>>
  result<size_t> write(It first, Se last) {
    if (const auto n = std::ranges::distance(first, last); n == 0) return 0;
    else if (n < 0) return fail<size_t>(errors::invalid_argument, "invalid iterator range");
    else return write(std::to_address(first), static_cast<size_t>(n) * sizeof(iter_value_t<It>));
  }

  template<contiguous_range R> requires trivial<iter_value_t<R>> result<size_t> write(R&& range) {
    return write(std::ranges::data(range), std::ranges::size(range) * sizeof(iter_value_t<R>));
  }

  result<void> write_exact(const void* src, size_t bytes) {
    const auto p = static_cast<const std::byte*>(src);
    for (size_t total = 0; total < bytes;) {
      if (auto res = write(p + total, bytes - total); !res) return res.relay_error<void>();
      else if (*res == 0) return fail<void>(errors::operation_failed, "incomplete write");
      else total += *res;
    }
    return {};
  }

  template<contiguous_iterator It, sized_sentinel_for<It> Se> requires trivial<iter_value_t<It>>
  result<void> write_exact(It first, Se last) {
    if (first >= last) return fail<void>(errors::invalid_argument, "invalid iterator range");
    return write_exact(std::to_address(first), std::ranges::distance(first, last) * sizeof(iter_value_t<It>));
  }

  template<contiguous_range R> requires trivial<iter_value_t<R>> result<void> write_exact(R&& range) {
    return write_exact(std::ranges::data(range), std::ranges::size(range) * sizeof(iter_value_t<R>));
  }

  template<trivial T> result<void> write_trivial(const T& v) {
    if (auto res = write_exact(&v, sizeof(T))) return {};
    else return res.relay();
  }

  template<typename T> requires is_bounded_array<T> && same_as<iter_value_t<T>, char>
  result<void> write_literal(const T& arr) {
    return write_exact(arr, (arraysize(arr) - 1) * sizeof(char));
  }

  result<void> flush() {
    if (!_file) return fail<void>(errors::not_initialized);
    if (std::fflush(_file) != 0) return fail<void>(errors::operation_failed, "flush error", errno);
    return {};
  }

  result<void> truncate_to_current() {
    if (!_file) return fail<void>(errors::not_initialized);
    if (auto res = internal::_truncate(_file)) return {};
    else return res.relay();
  }

  result<void> close_at_current() {
    if (auto res = truncate_to_current(); !res) return res.relay();
    if (auto res = close(); !res) return res.relay();
    return {};
  }
};

inline handle open(stringable auto&& Path, open_mode m, const source_line& sl = here()) {
  if (auto res = handle::create(static_cast<decltype(Path)&&>(Path), m); !res) {
    res.error().add_footprint().fizzle_out(sl);
    return {};
  } else return move(*res);
}
} // namespace yw::file
