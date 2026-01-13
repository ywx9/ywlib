#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>

namespace yw {

class reader;

inline reader

class reader {
  std::FILE* _file = nullptr;
  std::filesystem::path _path;

  void _close_noexcept() noexcept {
    if (!_file) return;
    std::fclose(std::exchange(_file, nullptr));
  }

  static std::FILE* _open_file(const std::filesystem::path& p) {
#ifdef _WIN32
    return ::_wfopen(p.c_str(), L"rb");
#else
    return std::fopen(p.c_str(), "rb");
#endif
  }

  static std::int64_t _tell(std::FILE* f) {
#ifdef _WIN32
    return ::_ftelli64(f);
#else
    return ::ftello(f);
#endif
  }

  static bool _seek(std::FILE* f, std::int64_t off, whence w) {
#ifdef _WIN32
    return ::_fseeki64(f, static_cast<__int64>(off), static_cast<int>(w)) == 0;
#else
    return ::fseeko(f, static_cast<off_t>(off), static_cast<int>(w)) == 0;
#endif
  }
public:
  using offset_type = std::int64_t;

  enum class whence { begin = SEEK_SET, current = SEEK_CUR, end = SEEK_END };

  // path を受け取ってファイルを開く（バイナリ読み）
  explicit reader(const std::filesystem::path& path) : _file(open_file(path)), _path(path) {
    if (!_file) throw std::runtime_error("yw::reader: failed to open file");
  }

  ~reader() { close_noexcept(); }
  reader() = default;

  reader(const reader&) = delete;
  reader& operator=(const reader&) = delete;

  reader(reader&& other) noexcept
    : _file(std::exchange(other._file, nullptr)), _path(std::move(other._path)) {}

  reader& operator=(reader&& other) noexcept {
    if (this != &other) {
      close_noexcept();
      _file = std::exchange(other._file, nullptr);
      _path = std::move(other._path);
    }
    return *this;
  }

  // 開けているか
  bool is_open() const noexcept { return _file != nullptr; }

  // パス（デバッグ用）
  const std::filesystem::path& path() const noexcept { return _path; }

  // 現在位置
  offset_type tell() const {
    ensure_open();
    return tell_impl(_file);
  }

  // シーク
  void seek(offset_type off, whence w = whence::begin) {
    ensure_open();
    if (!seek_impl(_file, off, w)) throw std::runtime_error("yw::reader: seek failed");
  }

  // ファイルサイズ（元の位置は保持）
  offset_type size() const {
    ensure_open();
    const auto cur = tell();
    if (!seek_impl(_file, 0, whence::end))
      throw std::runtime_error("yw::reader: seek(end) failed");
    const auto endpos = tell();
    if (!seek_impl(_file, cur, whence::begin))
      throw std::runtime_error("yw::reader: seek(restore) failed");
    return endpos;
  }

  // 読めた分だけ返す（0もあり得る: EOF）
  std::size_t read(void* dst, std::size_t bytes) {
    ensure_open();
    if (bytes == 0) return 0;
    return std::fread(dst, 1, bytes, _file);
  }

  // 指定バイト数ちょうど読めないと例外（EOF/エラー）
  void read_exact(void* dst, std::size_t bytes) {
    ensure_open();
    std::byte* p = static_cast<std::byte*>(dst);
    std::size_t total = 0;
    while (total < bytes) {
      const std::size_t n = std::fread(p + total, 1, bytes - total, _file);
      if (n == 0) {
        if (std::ferror(_file)) throw std::runtime_error("yw::reader: read error");
        throw std::runtime_error("yw::reader: unexpected EOF");
      }
      total += n;
    }
  }

  // 便利：PODを読む（必要なら）
  template<class T> requires std::is_trivially_copyable_v<T>
  T read_trivial() {
    T v{};
    read_exact(&v, sizeof(T));
    return v;
  }

  std::string read_as_string(std::size_t bytes = std::string::npos) {
    ensure_open();
    if (bytes == std::string::npos) bytes = static_cast<std::size_t>(size() - tell());
    std::string s(bytes, '\0');
    read_exact(s.data(), bytes);
    return s;
  }

  // 明示close（2回呼んでもOK）
  void close() {
    close_noexcept();
    _file = nullptr;
  }
};

} // namespace yw
