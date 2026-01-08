#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <utility>

#ifdef _WIN32
#include <io.h> // _fileno, _chsize_s
#else
#include <unistd.h> // fileno, ftruncate
#endif

namespace yw {

class writer {
public:
  using offset_type = std::int64_t;

  enum class whence { begin = SEEK_SET, current = SEEK_CUR, end = SEEK_END };

  enum class open_mode {
    truncate,        // wb   : create/overwrite
    append,          // ab+  : append (read/write)
    update,          // rb+  : read/write existing file
    update_or_create // rb+ or wb+ : read/write existing file or create new file
  };

  explicit writer(const std::filesystem::path& path, open_mode mode = open_mode::truncate)
    : _file(open_file(path, mode)), _path(path), _mode(mode) {
    if (!_file) { throw std::runtime_error("yw::writer: failed to open file"); }
  }

  ~writer() { close_noexcept(); }
  writer() noexcept = default;

  writer(const writer&) = delete;
  writer& operator=(const writer&) = delete;

  writer(writer&& other) noexcept
    : _file(std::exchange(other._file, nullptr)), _path(std::move(other._path)),
      _mode(other._mode) {}

  writer& operator=(writer&& other) noexcept {
    if (this != &other) {
      close_noexcept();
      _file = std::exchange(other._file, nullptr);
      _path = std::move(other._path);
      _mode = other._mode;
    }
    return *this;
  }

  bool is_open() const noexcept { return _file != nullptr; }
  const std::filesystem::path& path() const noexcept { return _path; }

  // ----------------------------------------------------------
  // position
  // ----------------------------------------------------------
  offset_type tell() const {
    ensure_open();
    return tell_impl(_file);
  }

  void seek(offset_type off, whence w = whence::begin) {
    ensure_open();
    if (!seek_impl(_file, off, w)) { throw std::runtime_error("yw::writer: seek failed"); }
  }

  offset_type size() const {
    ensure_open();
    const auto cur = tell();
    if (!seek_impl(_file, 0, whence::end)) throw std::runtime_error("yw::writer: seek(end) failed");
    const auto endpos = tell();
    if (!seek_impl(_file, cur, whence::begin))
      throw std::runtime_error("yw::writer: seek(restore) failed");
    return endpos;
  }

  // ----------------------------------------------------------
  // write
  // ----------------------------------------------------------
  std::size_t write(const void* src, std::size_t bytes) {
    ensure_open();
    if (bytes == 0) return 0;
    return std::fwrite(src, 1, bytes, _file);
  }

  void write_exact(const void* src, std::size_t bytes) {
    ensure_open();
    const std::byte* p = static_cast<const std::byte*>(src);
    std::size_t total = 0;
    while (total < bytes) {
      const std::size_t n = std::fwrite(p + total, 1, bytes - total, _file);
      if (n == 0) throw std::runtime_error("yw::writer: write error");
      total += n;
    }
  }

  template<class T> void write_trivial(const T& v) {
    static_assert(std::is_trivially_copyable_v<T>);
    write_exact(&v, sizeof(T));
  }

  // ----------------------------------------------------------
  // read (update / append モード用)
  // ----------------------------------------------------------
  std::size_t read(void* dst, std::size_t bytes) {
    ensure_open();
    if (bytes == 0) return 0;
    return std::fread(dst, 1, bytes, _file);
  }

  void read_exact(void* dst, std::size_t bytes) {
    ensure_open();
    std::byte* p = static_cast<std::byte*>(dst);
    std::size_t total = 0;
    while (total < bytes) {
      const std::size_t n = std::fread(p + total, 1, bytes - total, _file);
      if (n == 0) throw std::runtime_error("yw::writer: read error or EOF");
      total += n;
    }
  }

  template<class T> T read_trivial() {
    static_assert(std::is_trivially_copyable_v<T>);
    T v{};
    read_exact(&v, sizeof(T));
    return v;
  }

  // ----------------------------------------------------------
  // misc
  // ----------------------------------------------------------
  void flush() {
    ensure_open();
    if (std::fflush(_file) != 0) throw std::runtime_error("yw::writer: flush failed");
  }

  void close() {
    close_noexcept();
    _file = nullptr;
  }

  void truncate_to_current() {
    ensure_open();
    // 念のためバッファを吐く（サイズ・位置整合を取りやすくする）
    if (std::fflush(_file) != 0) throw std::runtime_error("yw::writer: flush failed");

    const auto cur = tell(); // offset_type (int64)
    if (cur < 0) throw std::runtime_error("yw::writer: invalid tell position");

#ifdef _WIN32
    const int fd = ::_fileno(_file);
    if (fd < 0) throw std::runtime_error("yw::writer: _fileno failed");

    if (::_chsize_s(fd, static_cast<__int64>(cur)) != 0)
      throw std::runtime_error("yw::writer: truncate failed");
#else
    const int fd = ::fileno(_file);
    if (fd < 0) throw std::runtime_error("yw::writer: fileno failed");

    if (::ftruncate(fd, static_cast<off_t>(cur)) != 0)
      throw std::runtime_error("yw::writer: truncate failed");
#endif
  }

  void close_at_current() {
    truncate_to_current();
    close();
  }

private:
  std::FILE* _file = nullptr;
  std::filesystem::path _path;
  open_mode _mode = open_mode::truncate;

  void ensure_open() const {
    if (!_file) throw std::runtime_error("yw::writer: file not open");
  }

  void close_noexcept() noexcept {
    if (_file) {
      std::fclose(_file);
      _file = nullptr;
    }
  }

  static std::FILE* open_file(const std::filesystem::path& p, open_mode mode) {
#ifdef _WIN32
    auto wfopen = [&](const wchar_t* m) -> std::FILE* { return ::_wfopen(p.c_str(), m); };
    switch (mode) {
    case open_mode::truncate: return wfopen(L"wb");
    case open_mode::append: return wfopen(L"ab+");
    case open_mode::update: return wfopen(L"rb+");
    case open_mode::update_or_create: {
      if (auto* f = wfopen(L"rb+")) return f;
      return wfopen(L"wb+");
    }
    }
    return nullptr; // unreachable
#else
    auto fopen_ = [&](const char* m) -> std::FILE* { return std::fopen(p.c_str(), m); };
    switch (mode) {
    case open_mode::truncate: return fopen_("wb");
    case open_mode::append: return fopen_("ab+");
    case open_mode::update: return fopen_("rb+");
    case open_mode::update_or_create: {
      if (auto* f = fopen_("rb+")) return f;
      return fopen_("wb+");
    }
    }
    return nullptr; // unreachable
#endif
  }

  static offset_type tell_impl(std::FILE* f) {
#ifdef _WIN32
    const auto pos = ::_ftelli64(f);
#else
    const auto pos = ::ftello(f);
#endif
    if (pos < 0) throw std::runtime_error("yw::writer: tell failed");
    return static_cast<offset_type>(pos);
  }

  static bool seek_impl(std::FILE* f, offset_type off, whence w) {
#ifdef _WIN32
    return ::_fseeki64(f, static_cast<__int64>(off), static_cast<int>(w)) == 0;
#else
    return ::fseeko(f, static_cast<off_t>(off), static_cast<int>(w)) == 0;
#endif
  }
};

} // namespace yw
