#pragma once
#include <yw/datetime.h>
#include <yw/error.h>
#include <yw/null_terminated.h>

namespace yw::file {

#ifdef _WIN32
using path_char = wchar_t;
using path_string = string<wchar_t>;
#else
using path_char = char;
using path_string = string<char>;
#endif

enum class kind {
  unknown,
  regular,
  directory,
  symlink,
  other,
};

struct info {
  kind kind = kind::unknown;
  uint64_t size = 0; // regular file only
  bool exists = false;
  bool read_only = false;
  datetime last_write_time{{0, 0, 0}, {0, 0, 0}};
};

#ifdef _WIN32
inline constexpr path_char preferred_separator = L'\\';
#else
inline constexpr path_char preferred_separator = '/';
#endif

namespace internal {
inline constexpr auto is_separator = []<char_type C>(C c) noexcept {
#ifdef _WIN32
  return c == C('/') || c == C('\\');
#else
  return c == C('/');
#endif
};

template<char_type C> constexpr auto filename(string_view<C> sv) noexcept {
  for (size_t i = sv.size(); i != 0; --i)
    if (is_separator(sv[i - 1])) return string_view<C>(sv.data() + i, sv.size() - i);
  return sv;
}

template<char_type C> constexpr auto parent(string_view<C> sv) noexcept {
  for (size_t i = sv.size(); i != 0; --i)
    if (is_separator(sv[i - 1])) return i == 1 ? string_view<C>(sv.data(), 1) : string_view<C>(sv.data(), i - 1);
  return string_view<C>();
}

template<char_type C> constexpr auto extension_position(string_view<C> sv) noexcept {
  const auto name = filename(sv);
  for (size_t i = name.size(); i > 1; --i)
    if (name[i - 1] == C('.')) return size_t(name.data() - sv.data()) + i - 1;
  return npos;
}
template<char_type C> constexpr auto extension(string_view<C> sv) noexcept {
  const auto pos = extension_position(sv);
  if (pos == npos) return string_view<C>();
  return string_view<C>(sv.data() + pos, sv.size() - pos);
}

template<char_type C> constexpr auto stem(string_view<C> sv) noexcept {
  const auto name = filename(sv);
  const auto pos = extension_position(name);
  if (pos == npos) return name;
  return string_view<C>(name.data(), pos);
}

template<char_type C> constexpr auto replace_extension(string_view<C> sv, string_view<C> new_ext) noexcept {
  const auto pos = extension_position(sv);
  const auto len = pos == npos ? sv.size() : pos;
  const bool add_dot = !new_ext.empty() && new_ext[0] != C('.');
  string<C> result;
  result.reserve(len + add_dot + new_ext.size());
  result.append(sv.data(), len);
  if (add_dot) result.push_back(C('.'));
  result.append(new_ext.data(), new_ext.size());
  return result;
}

template<char_type C> constexpr size_t root_length(string_view<C> sv) noexcept {
  if (sv.empty()) return 0;
#ifdef _WIN32
  // Drive root: C:\ or C:/
  if (sv.size() >= 3 && ((sv[0] >= C('A') && sv[0] <= C('Z')) || (sv[0] >= C('a') && sv[0] <= C('z'))) &&
      sv[1] == C(':') && is_separator(sv[2]))
    return 3;
  // UNC root: \\server\share\ or //server/share/
  if (sv.size() >= 2 && is_separator(sv[0]) && is_separator(sv[1])) {
    size_t i = 2;
    while (i < sv.size() && is_separator(sv[i])) ++i;
    while (i < sv.size() && !is_separator(sv[i])) ++i;
    while (i < sv.size() && is_separator(sv[i])) ++i;
    while (i < sv.size() && !is_separator(sv[i])) ++i;
    if (i < sv.size() && is_separator(sv[i])) return i + 1;
    return i;
  }
#endif
  // Root-relative path: \foo
  return is_separator(sv[0]);
}
} // namespace internal

template<stringable S> constexpr auto filename(S&& Path) noexcept {
  return internal::filename(string_view<iter_value_t<S>>(Path));
}

template<stringable S> constexpr auto parent(S&& Path) noexcept {
  return internal::parent(string_view<iter_value_t<S>>(Path));
}

template<stringable S> constexpr auto extension(S&& Path) noexcept {
  return internal::extension(string_view<iter_value_t<S>>(Path));
}

template<stringable S> constexpr auto stem(S&& Path) noexcept {
  return internal::stem(string_view<iter_value_t<S>>(Path));
}

template<stringable S> constexpr bool has_extension(S&& Path) noexcept {
  return internal::extension_position(string_view<iter_value_t<S>>(Path)) != npos;
}

template<stringable S, stringable<iter_value_t<S>> E> constexpr auto replace_extension(S&& Path, E&& NewExt) noexcept {
  return internal::replace_extension(string_view<iter_value_t<S>>(Path), string_view<iter_value_t<S>>(NewExt));
}

template<char Sep, stringable S> requires(Sep == '/' || Sep == '\\')
constexpr auto normalize_separators(S&& Path) noexcept {
  constexpr auto sep = static_cast<iter_value_t<S>>(Sep);
  const auto sv = string_view<iter_value_t<S>>(Path);
  string<iter_value_t<S>> result;
  result.reserve(Path.size());
  bool skip = false;
  for (const auto c : sv)
    if (internal::is_separator(c)) {
      if (!skip) result.push_back(sep);
      skip = true;
    } else result.push_back(c), skip = false;
  return result;
}

template<typename... Ss> requires ((stringable<Ss> || char_type<Ss>) && ...)
constexpr auto join(Ss&&... Paths) noexcept {
  if constexpr (sizeof...(Paths) == 0) return string<path_char>();
  else {
    string<path_char> result = format<path_char>(select<0>(static_cast<Ss&&>(Paths)...));
    auto f = [&]<size_t I>(constant<I>) {
      result.push_back(preferred_separator);
      result.append(format<path_char>(select<I>(static_cast<Ss&&>(Paths)...)));
    };
    [&]<size_t... Is>(sequence<Is...>) { (f(constant<Is>{}), ...); }(make_sequence<1, sizeof...(Paths)>());
    return result;
  }
}

namespace internal {
#ifdef _WIN32
inline datetime to_datetime(FILETIME ft) noexcept {
  constexpr uint64_t epoch = 116444736000000000ull;
  ULARGE_INTEGER v{.LowPart = ft.dwLowDateTime, .HighPart = ft.dwHighDateTime};
  if (v.QuadPart < epoch) return {};
  const auto duration = std::chrono::duration<uint64_t, std::ratio<1, 10000000>>(v.QuadPart - epoch);
  return datetime(
    std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::system_clock::duration>(duration)));
}
inline std::expected<info, error> _query_info(const wchar_t* p) {
  WIN32_FILE_ATTRIBUTE_DATA data;
  if (!::GetFileAttributesExW(p, GetFileExInfoStandard, &data)) {
    const auto ec = ::GetLastError();
    if (ec == ERROR_FILE_NOT_FOUND || ec == ERROR_PATH_NOT_FOUND || ec == ERROR_INVALID_NAME) return info();
    return std::unexpected(error(errors::operation_failed, "GetFileAttributesExW failed", static_cast<int32_t>(ec)));
  }
  info result;
  result.exists = true;
  result.read_only = (data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
  if (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) result.kind = kind::symlink;
  else if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) result.kind = kind::directory;
  else result.kind = kind::regular;
  if (result.kind == kind::regular) result.size = (uint64_t(data.nFileSizeHigh) << 32) | uint64_t(data.nFileSizeLow);
  result.last_write_time = to_datetime(data.ftLastWriteTime);
  return result;
}
#else
inline datetime to_datetime(const timespec& ts) noexcept {
  return datetime(
    std::chrono::system_clock::time_point(std::chrono::seconds(ts.tv_sec) + std::chrono::nanoseconds(ts.tv_nsec)));
}
inline std::expected<info, error> _query_info(const char* p) {
  struct stat st;
  if (::stat(p, &st) != 0) {
    if (errno == ENOENT || errno == ENOTDIR) return info();
    return std::unexpected(error(errors::operation_failed, "stat failed", errno));
  }
  info result;
  result.exists = true;
  result.read_only = (st.st_mode & (S_IWUSR | S_IWGRP | S_IWOTH)) == 0;
  if (S_ISREG(st.st_mode)) result.kind = kind::regular;
  else if (S_ISDIR(st.st_mode)) result.kind = kind::directory;
  else if (S_ISLNK(st.st_mode)) result.kind = kind::symlink;
  else result.kind = kind::other;
  if (result.kind == kind::regular) result.size = static_cast<uint64_t>(st.st_size);
#ifdef __APPLE__
  result.last_write_time = to_datetime(st.st_mtimespec);
#else
  result.last_write_time = to_datetime(st.st_mtim);
#endif
  return result;
}
#endif
inline std::expected<info, error> query_info(null_terminated<path_char> Path) {
  return _query_info(Path.data());
}
} // namespace internal

inline info stat(stringable auto&& Path) {
  if (auto res = internal::query_info(static_cast<decltype(Path)&&>(Path)); !res) {
    res.error().add_footprint().fizzle_out();
    return {};
  } else return *res;
}

inline bool exists(stringable auto&& Path) {
  if (auto res = internal::query_info(static_cast<decltype(Path)&&>(Path)); !res) {
    res.error().add_footprint().fizzle_out();
    return false;
  } else return res->exists;
}

inline bool is_file(stringable auto&& Path) {
  if (auto res = internal::query_info(static_cast<decltype(Path)&&>(Path)); !res) {
    res.error().add_footprint().fizzle_out();
    return false;
  } else return res->kind == kind::regular;
}

inline bool is_directory(stringable auto&& Path) {
  if (auto res = internal::query_info(static_cast<decltype(Path)&&>(Path)); !res) {
    res.error().add_footprint().fizzle_out();
    return false;
  } else return res->kind == kind::directory;
}

inline uint64_t size(stringable auto&& Path) {
  if (auto res = internal::query_info(static_cast<decltype(Path)&&>(Path)); !res) {
    res.error().add_footprint().fizzle_out();
  } else if (res->kind == kind::regular) return res->size;
  return 0;
}

inline datetime last_write_time(stringable auto&& Path) {
  if (auto res = internal::query_info(static_cast<decltype(Path)&&>(Path)); !res) {
    res.error().add_footprint().fizzle_out();
    return {};
  } else return res->last_write_time;
}

namespace internal {
#ifdef _WIN32
inline std::expected<void, error> rename(const wchar_t* from, const wchar_t* to) {
  if (::MoveFileExW(from, to, MOVEFILE_REPLACE_EXISTING)) return {};
  return std::unexpected(error(errors::operation_failed, "MoveFileExW failed", int32_t(::GetLastError())));
}

inline std::expected<void, error> copy_file(const wchar_t* from, const wchar_t* to, bool overwrite) {
  if (::CopyFileW(from, to, overwrite ? FALSE : TRUE)) return {};
  const auto ec = ::GetLastError();
  if (!overwrite && (ec == ERROR_FILE_EXISTS || ec == ERROR_ALREADY_EXISTS)) return {};
  return std::unexpected(error(errors::operation_failed, "CopyFileW failed", int32_t(ec)));
}

inline std::expected<void, error> remove_file(const wchar_t* p) {
  if (::DeleteFileW(p)) return {};
  const auto ec = ::GetLastError();
  if (ec == ERROR_FILE_NOT_FOUND || ec == ERROR_PATH_NOT_FOUND) return {};
  return std::unexpected(error(errors::operation_failed, "DeleteFileW failed", int32_t(ec)));
}

inline std::expected<void, error> resize_file(const wchar_t* p, uint64_t size) {
  if (size > uint64_t(std::numeric_limits<LONGLONG>::max()))
    return std::unexpected(error(errors::invalid_argument, "file size is too large"));
  const auto h = ::CreateFileW(
    p, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL, nullptr);
  if (h == INVALID_HANDLE_VALUE)
    return std::unexpected(error(errors::operation_failed, "CreateFileW failed", int32_t(::GetLastError())));

  LARGE_INTEGER pos;
  pos.QuadPart = static_cast<LONGLONG>(size);
  if (!::SetFilePointerEx(h, pos, nullptr, FILE_BEGIN)) {
    const auto ec = ::GetLastError();
    ::CloseHandle(h);
    return std::unexpected(error(errors::operation_failed, "SetFilePointerEx failed", int32_t(ec)));
  }
  if (!::SetEndOfFile(h)) {
    const auto ec = ::GetLastError();
    ::CloseHandle(h);
    return std::unexpected(error(errors::operation_failed, "SetEndOfFile failed", int32_t(ec)));
  }
  if (!::CloseHandle(h))
    return std::unexpected(error(errors::operation_failed, "CloseHandle failed", int32_t(::GetLastError())));
  return {};
}

inline std::expected<void, error> create_directory(const wchar_t* p) {
  if (::CreateDirectoryW(p, nullptr)) return {};
  const auto ec = ::GetLastError();
  if (ec == ERROR_ALREADY_EXISTS) {
    const auto attributes = ::GetFileAttributesW(p);
    if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY)) return {};
  }
  return std::unexpected(error(errors::operation_failed, "CreateDirectoryW failed", int32_t(ec)));
}

inline std::expected<void, error> remove_directory(const wchar_t* p) {
  if (::RemoveDirectoryW(p)) return {};
  const auto ec = ::GetLastError();
  if (ec == ERROR_FILE_NOT_FOUND || ec == ERROR_PATH_NOT_FOUND) return {};
  return std::unexpected(error(errors::operation_failed, "RemoveDirectoryW failed", int32_t(ec)));
}

#else

inline std::expected<void, error> rename(const char* from, const char* to) {
  if (::rename(from, to) == 0) return {};
  return std::unexpected(error(errors::operation_failed, "rename failed", errno));
}

inline std::expected<void, error> copy_file(const char* from, const char* to, bool overwrite) {
  const int src = ::open(from, O_RDONLY);
  if (src == -1) return std::unexpected(error(errors::operation_failed, "open source failed", errno));

  struct stat st;
  if (::fstat(src, &st) != 0) {
    const auto ec = errno;
    ::close(src);
    return std::unexpected(error(errors::operation_failed, "fstat failed", ec));
  }

  const int flags = O_WRONLY | O_CREAT | (overwrite ? O_TRUNC : O_EXCL);
  const int dst = ::open(to, flags, st.st_mode & 0777);
  if (dst == -1) {
    const auto ec = errno;
    ::close(src);
    if (!overwrite && ec == EEXIST) return {};
    return std::unexpected(error(errors::operation_failed, "open destination failed", ec));
  }

  std::byte buffer[65536];
  for (;;) {
    const auto n = ::read(src, buffer, sizeof(buffer));
    if (n == 0) break;
    if (n < 0) {
      if (errno == EINTR) continue;
      const auto ec = errno;
      ::close(dst);
      ::close(src);
      ::unlink(to);
      return std::unexpected(error(errors::operation_failed, "read failed", ec));
    }

    ssize_t written = 0;
    while (written < n) {
      const auto m = ::write(dst, buffer + written, size_t(n - written));
      if (m < 0) {
        if (errno == EINTR) continue;
        const auto ec = errno;
        ::close(dst);
        ::close(src);
        ::unlink(to);
        return std::unexpected(error(errors::operation_failed, "write failed", ec));
      }
      written += m;
    }
  }

  if (::close(dst) != 0) {
    const auto ec = errno;
    ::close(src);
    return std::unexpected(error(errors::operation_failed, "close destination failed", ec));
  }
  if (::close(src) != 0) return std::unexpected(error(errors::operation_failed, "close source failed", errno));
  return {};
}

inline std::expected<void, error> remove_file(const char* p) {
  if (::unlink(p) == 0 || errno == ENOENT) return {};
  return std::unexpected(error(errors::operation_failed, "unlink failed", errno));
}

inline std::expected<void, error> resize_file(const char* p, uint64_t size) {
  if (size > uint64_t(std::numeric_limits<off_t>::max()))
    return std::unexpected(error(errors::invalid_argument, "file size is too large"));
  if (::truncate(p, static_cast<off_t>(size)) == 0) return {};
  return std::unexpected(error(errors::operation_failed, "truncate failed", errno));
}

inline std::expected<void, error> create_directory(const char* p) {
  if (::mkdir(p, 0777) == 0) return {};

  const auto ec = errno;
  if (ec == EEXIST) {
    struct stat st;
    if (::stat(p, &st) == 0 && S_ISDIR(st.st_mode)) return {};
  }
  return std::unexpected(error(errors::operation_failed, "mkdir failed", ec));
}

inline std::expected<void, error> remove_directory(const char* p) {
  if (::rmdir(p) == 0 || errno == ENOENT) return {};
  return std::unexpected(error(errors::operation_failed, "rmdir failed", errno));
}

#endif
} // namespace internal

inline std::expected<void, error> rename(null_terminated<path_char> From, null_terminated<path_char> To) {
  if (auto res = internal::rename(From.data(), To.data())) return {};
  else return res.error().relay();
}

inline std::expected<void, error> copy_file(null_terminated<path_char> From, null_terminated<path_char> To, bool overwrite = false) {
  if (auto res = internal::copy_file(From.data(), To.data(), overwrite)) return {};
  else return res.error().relay();
}

inline std::expected<void, error> remove_file(null_terminated<path_char> Path) {
  if (auto res = internal::remove_file(Path.data())) return {};
  else return res.error().relay();
}

inline std::expected<void, error> resize_file(null_terminated<path_char> Path, uint64_t size) {
  if (auto res = internal::resize_file(Path.data(), size)) return {};
  else return res.error().relay();
}

inline std::expected<void, error> create_directory(null_terminated<path_char> Path) {
  if (auto res = internal::create_directory(Path.data())) return {};
  else return res.error().relay();
}

inline std::expected<void, error> remove_directory(null_terminated<path_char> Path) {
  if (auto res = internal::remove_directory(Path.data())) return {};
  else return res.error().relay();
}

inline std::expected<void, error> create_directories(stringable auto Path) {
  auto p = unicode<path_char>(static_cast<decltype(Path)&&>(Path));
  const auto root = internal::root_length(string_view<path_char>(p.data(), p.size()));
  for (size_t i = root; i < p.size(); ++i) {
    if (!internal::is_separator(p[i])) continue;
    const auto separator = std::exchange(p[i], path_char());
    if (i != 0)
      if (auto res = internal::create_directory(p.data()); !res) return res.error().relay();
    p[i] = separator;
    while (i + 1 < p.size() && internal::is_separator(p[i + 1])) ++i;
  }
  if (p.size() != root)
    if (auto res = internal::create_directory(p.data()); !res) return res.error().relay();
  return {};
}

namespace internal {
template<char_type C> string<C> child_path(string_view<C> directory, string_view<C> name) {
  string<C> result;
  const bool add_separator = !directory.empty() && !is_separator(directory.back());
  result.reserve(directory.size() + size_t(add_separator) + name.size());
  result.append(directory);
  if (add_separator) {
#ifdef _WIN32
    result.push_back(C('\\'));
#else
    result.push_back(C('/'));
#endif
  }
  result.append(name);
  return result;
}

#ifdef _WIN32
inline std::expected<void, error> list_files(
  const wchar_t* directory, bool recursive, std::vector<string<path_char>>& result) {
  const string_view<wchar_t> dir(directory);
  auto pattern = child_path<wchar_t>(dir, L"*");
  WIN32_FIND_DATAW data;
  const auto find = ::FindFirstFileW(pattern.data(), &data);
  if (find == INVALID_HANDLE_VALUE) {
    const auto ec = ::GetLastError();
    if (ec == ERROR_FILE_NOT_FOUND) return {};
    return std::unexpected(error(errors::operation_failed, "FindFirstFileW failed", int32_t(ec)));
  }
  do {
    const string_view<wchar_t> name(data.cFileName);
    if (name == L"." || name == L"..") continue;
    auto path = child_path(dir, name);
    if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      if (recursive && !(data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
        if (auto res = list_files(path.data(), true, result); !res) {
          ::FindClose(find);
          return res.error().relay();
        }
    } else result.push_back(std::move(path));
  } while (::FindNextFileW(find, &data));
  const auto ec = ::GetLastError();
  ::FindClose(find);
  if (ec != ERROR_NO_MORE_FILES)
    return std::unexpected(error(errors::operation_failed, "FindNextFileW failed", int32_t(ec)));
  return {};
}
#else
inline std::expected<void, error> list_files(
  const char* directory, bool recursive, std::vector<string<path_char>>& result) {
  const string_view<char> dirpath(directory);
  auto dir = ::opendir(directory);
  if (!dir) return std::unexpected(error(errors::operation_failed, "opendir failed", errno));
  errno = 0;
  while (const auto entry = ::readdir(dir)) {
    const string_view<char> name(entry->d_name);
    if (name == "." || name == "..") continue;
    auto path = child_path(dirpath, name);
    struct stat st;
    if (::lstat(path.data(), &st) != 0) {
      const auto ec = errno;
      ::closedir(dir);
      return std::unexpected(error(errors::operation_failed, "lstat failed", ec));
    }
    if (S_ISREG(st.st_mode)) result.push_back(std::move(path));
    else if (recursive && S_ISDIR(st.st_mode))
      if (auto res = list_files(path.data(), true, result); !res) {
        ::closedir(dir);
        return res.error().relay();
      }
  }
  const auto ec = errno;
  if (::closedir(dir) != 0) return std::unexpected(error(errors::operation_failed, "closedir failed", errno));
  if (ec != 0) return std::unexpected(error(errors::operation_failed, "readdir failed", ec));
  return {};
}
#endif
} // namespace internal

inline std::vector<string<path_char>> list_files(
  null_terminated<path_char> Directory, bool recursive = false, const source_line& sl = here()) {
  std::vector<string<path_char>> result;
  if (auto res = internal::list_files(Directory.data(), recursive, result); !res)
    res.error().add_footprint().fizzle_out(sl);
  return result;
}

/// MARK: relative

namespace internal {
template<char_type C> constexpr bool path_char_equal(C a, C b) noexcept {
  if (is_separator(a) && is_separator(b)) return true;
#ifdef _WIN32
  if (a >= C('A') && a <= C('Z')) a += C('a') - C('A');
  if (b >= C('A') && b <= C('Z')) b += C('a') - C('A');
#endif
  return a == b;
}

template<char_type C> constexpr auto next_component(string_view<C> sv, size_t& pos) noexcept {
  while (pos < sv.size() && is_separator(sv[pos])) ++pos;
  const auto begin = pos;
  while (pos < sv.size() && !is_separator(sv[pos])) ++pos;
  return string_view<C>(sv.data() + begin, pos - begin);
}

template<char_type C> constexpr bool component_equal(string_view<C> a, string_view<C> b) noexcept {
  if (a.size() != b.size()) return false;
  for (size_t i = 0; i < a.size(); ++i)
    if (!path_char_equal(a[i], b[i])) return false;
  return true;
}

template<char_type C> auto relative(string_view<C> path, string_view<C> base) {
  const auto path_root = root_length(path);
  const auto base_root = root_length(base);
  if (path_root != base_root || !component_equal(path.substr(0, path_root), base.substr(0, base_root)))
    return string<C>();
  size_t path_pos = path_root, base_pos = base_root, common_path_pos = path_pos, common_base_pos = base_pos;
  for (;;) {
    const auto old_path_pos = path_pos, old_base_pos = base_pos;
    const auto path_component = next_component(path, path_pos), base_component = next_component(base, base_pos);
    if (path_component.empty() || base_component.empty() || !component_equal(path_component, base_component)) {
      path_pos = old_path_pos, base_pos = old_base_pos;
      break;
    } else common_path_pos = path_pos, common_base_pos = base_pos;
  }
  path_pos = common_path_pos, base_pos = common_base_pos;
  size_t parent_count = 0;
  const auto dot_char = C('.');
  for (;;) {
    const auto component = next_component(base, base_pos);
    if (component.empty()) break;
    if (component != string_view<C>(&dot_char, 1)) ++parent_count;
  }
  while (path_pos < path.size() && is_separator(path[path_pos])) ++path_pos;
  string<C> result;
  constexpr auto separator =
#ifdef _WIN32
    C('\\');
#else
    C('/');
#endif
  for (size_t i = 0; i < parent_count; ++i) {
    if (!result.empty()) result.push_back(separator);
    result.push_back(C('.')), result.push_back(C('.'));
  }
  if (path_pos < path.size()) {
    if (!result.empty()) result.push_back(separator);
    for (; path_pos < path.size(); ++path_pos)
      result.push_back(is_separator(path[path_pos]) ? separator : path[path_pos]);
  }
  if (result.empty()) result.push_back(C('.'));
  return result;
}
} // namespace internal

template<stringable P, stringable<iter_value_t<P>> B> auto relative(P&& Path, B&& Base) {
  using C = iter_value_t<P>;
  return internal::relative(string_view<C>(Path), string_view<C>(Base));
}
} // namespace yw::file
