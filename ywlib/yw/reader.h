#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>

namespace yw {

class reader {
public:
  using offset_type = std::int64_t;

  enum class whence { begin = SEEK_SET, current = SEEK_CUR, end = SEEK_END };

  // path を受け取ってファイルを開く（バイナリ読み）
  explicit reader(const std::filesystem::path& path) : m_file(open_file(path)), m_path(path) {
    if (!m_file) throw std::runtime_error("yw::reader: failed to open file");
  }

  // RAII
  ~reader() { close_noexcept(); }

  reader(const reader&) = delete;
  reader& operator=(const reader&) = delete;

  reader(reader&& other) noexcept
    : m_file(std::exchange(other.m_file, nullptr)), m_path(std::move(other.m_path)) {}

  reader& operator=(reader&& other) noexcept {
    if (this != &other) {
      close_noexcept();
      m_file = std::exchange(other.m_file, nullptr);
      m_path = std::move(other.m_path);
    }
    return *this;
  }

  // 開けているか
  bool is_open() const noexcept { return m_file != nullptr; }

  // パス（デバッグ用）
  const std::filesystem::path& path() const noexcept { return m_path; }

  // 現在位置
  offset_type tell() const {
    ensure_open();
    return tell_impl(m_file);
  }

  // シーク
  void seek(offset_type off, whence w = whence::begin) {
    ensure_open();
    if (!seek_impl(m_file, off, w)) throw std::runtime_error("yw::reader: seek failed");
  }

  // ファイルサイズ（元の位置は保持）
  offset_type size() const {
    ensure_open();
    const auto cur = tell();
    if (!seek_impl(m_file, 0, whence::end))
      throw std::runtime_error("yw::reader: seek(end) failed");
    const auto endpos = tell();
    if (!seek_impl(m_file, cur, whence::begin))
      throw std::runtime_error("yw::reader: seek(restore) failed");
    return endpos;
  }

  // 読めた分だけ返す（0もあり得る: EOF）
  std::size_t read(void* dst, std::size_t bytes) {
    ensure_open();
    if (bytes == 0) return 0;
    return std::fread(dst, 1, bytes, m_file);
  }

  // 指定バイト数ちょうど読めないと例外（EOF/エラー）
  void read_exact(void* dst, std::size_t bytes) {
    ensure_open();
    std::byte* p = static_cast<std::byte*>(dst);
    std::size_t total = 0;
    while (total < bytes) {
      const std::size_t n = std::fread(p + total, 1, bytes - total, m_file);
      if (n == 0) {
        if (std::ferror(m_file)) throw std::runtime_error("yw::reader: read error");
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
    m_file = nullptr;
  }

private:
  std::FILE* m_file = nullptr;
  std::filesystem::path m_path;

  void ensure_open() const {
    if (!m_file) throw std::runtime_error("yw::reader: file not open");
  }

  void close_noexcept() noexcept {
    if (m_file) {
      std::fclose(m_file);
      m_file = nullptr;
    }
  }

  static std::FILE* open_file(const std::filesystem::path& p) {
#ifdef _WIN32
    // Windows: Unicode path を安全に扱うため _wfopen を使う
    // p.c_str() は wchar_t*（Windowsでは）
    return ::_wfopen(p.c_str(), L"rb");
#else
    // POSIX: path はバイト列（UTF-8が一般的）として fopen
    return std::fopen(p.c_str(), "rb");
#endif
  }

  static offset_type tell_impl(std::FILE* f) {
#ifdef _WIN32
    const auto pos = ::_ftelli64(f);
    if (pos < 0) throw std::runtime_error("yw::reader: tell failed");
    return static_cast<offset_type>(pos);
#else
    const auto pos = ::ftello(f);
    if (pos < 0) throw std::runtime_error("yw::reader: tell failed");
    return static_cast<offset_type>(pos);
#endif
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
