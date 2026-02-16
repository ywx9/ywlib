#pragma once

#include "yw/core.h"
#include "yw/file.h"

/**
 * \note
 * .ywaファイル
 * 用途: Simplestな非圧縮アーカイブ
 * 規則:
 * - 数値フィールドはすべてリトルエンディアン
 * - エントリ間、エントリ-フッタ間のパディングは許容されない
 * - ただし、エントリ開始前にデータがあっても動作を保証する
 * - エントリ名は2048バイト未満のUTF-8文字列
 * - フッタでのオフセットの並びはエントリの順序と対応する
 * - CRC32は各エントリのデータに対してIEEE 802.3準拠で計算する
 *   - (poly=0x04C11DB7, init=0xFFFFFFFF, refin=true, refout=true, xorout=0xFFFFFFFF)
 *   - "123456789" => 0xCBF43926
 * 構造:
 * - エントリ[N]
 *   - マジックナンバ(4) == YWAE; (0x45415759)
 *   - ファイル名の長さ(4)
 *   - データの長さ(8)
 *   - ファイル名
 *   - データ
 *   - CRC32(4)
 * - フッタ
 *   - マジックナンバ(4) == YWAF; (0x46415759)
 *   - ファイル数(4) == N
 *   - 各エントリのファイル先頭からのオフセット(8 x N)
 *   - フッタのファイル先頭からのオフセット(8)
 */

namespace yw::archive {

template<integral T> static constexpr T _to_le(T a) noexcept {
  if constexpr (std::endian::native == std::endian::little) return a;
  else return std::byteswap(a);
}

inline constexpr uint32_t entry_magic = 0x45415759;  // 'YWAE'
inline constexpr uint32_t footer_magic = 0x46415759; // 'YWAF'
inline constexpr uint32_t max_name_size = 2048;

struct entry {
  std::string name;
  uint64_t entry_offset;
  uint64_t data_offset;
  uint64_t data_length;
  uint32_t crc32;
};

struct footer {
  uint32_t magic;
  uint32_t entry_count;
};
static_assert(sizeof(footer) == 8);

struct header {
  uint32_t magic;
  uint32_t name_length;
  uint64_t data_length;
};
static_assert(sizeof(header) == 16);

class handle {
  file_handle _fh;
  std::vector<entry> _entries;
  uint64_t _entry_offset = 0, _footer_offset = 0;
  open_mode _mode = open_mode::unknown;
  explicit handle(file_handle&& fh, std::vector<entry>&& e, std::array<uint64_t, 2> offs, open_mode m)
    : _fh(std::move(fh)), _entries(std::move(e)), _entry_offset(offs[0]), _footer_offset(offs[1]), _mode(m) {}

public:
  /// creates `archive`
  static std::expected<handle, error_trace> create(const std::filesystem::path& p, open_mode mode) {
    const auto e = errors::invalid_file;
    const open_mode fh_mode = mode == open_mode::append ? open_mode::update_or_create : mode;
    auto fh = yw::open(p, fh_mode);
    if (!fh) return unexpected_error(fh.error());
    if (fh_mode == open_mode::create_always || fh_mode == open_mode::create_new)
      return handle(std::move(*fh), {}, {}, mode);
    uint64_t file_size{};
    if (auto res = fh->seek(0, seek_whence::end); !res) return unexpected_error(res.error());
    if (auto res = fh->tell(); !res) return unexpected_error(res.error());
    else file_size = static_cast<uint64_t>(*res);
    if (fh_mode == open_mode::update_or_create && file_size == 0) return handle(std::move(*fh), {}, {}, mode);
    /// \note need to check footer
    uint64_t footer_offset{};
    if (auto res = fh->seek(-8, seek_whence::end); !res) return unexpected_error(res.error());
    if (auto res = fh->read_trivial<uint64_t>(); !res) return unexpected_error(res.error());
    else footer_offset = _to_le(*res);
    if (footer_offset + sizeof(footer) + sizeof(uint64_t) > file_size)
      return unexpected_error(e, "archive: invalid footer offset", {}, file_size - 8);
    footer f;
    if (auto res = fh->seek(int64_t(footer_offset)); !res) return unexpected_error(res.error());
    if (auto res = fh->read_trivial(f); !res) return unexpected_error(res.error());
    else if (_to_le(f.magic) != footer_magic)
      return unexpected_error(e, "archive: invalid footer magic", {}, footer_offset);
    const uint32_t entry_count = _to_le(f.entry_count);
    const auto footer_size = sizeof(footer) + entry_count * sizeof(uint64_t) + sizeof(uint64_t);
    if (footer_offset + footer_size != file_size)
      return unexpected_error(e, "archive: invalid entry count", {}, footer_offset + 4);
    if (entry_count == 0) return handle(std::move(*fh), {}, {0, footer_offset}, mode);
    /// \note need to check entries
    std::vector<uint64_t> offsets(entry_count);
    if (auto res = fh->read_exact(offsets.data(), offsets.size() * sizeof(uint64_t)); !res)
      return unexpected_error(res.error());
    for (auto& off : offsets) off = _to_le(off);
    std::vector<entry> entries(entry_count);
    for (uint32_t i = 0; i < entry_count; ++i) {
      const auto off = offsets[i];
      if (off >= footer_offset)
        return unexpected_error(e, "archive: invalid entry offset", {}, footer_offset + 8 + i * 8);
      if (auto res = fh->seek(int64_t(off)); !res) return unexpected_error(res.error());
      header h;
      if (auto res = fh->read_trivial(h); !res) return unexpected_error(res.error());
      else if (_to_le(h.magic) != entry_magic) return unexpected_error(e, "archive: invalid entry magic", {}, off);
      const uint32_t name_length = _to_le(h.name_length);
      if (name_length == 0 || name_length > max_name_size)
        return unexpected_error(e, "archive: invalid name length", {}, off + 4);
      if (off + sizeof(header) + name_length > footer_offset)
        return unexpected_error(e, "archive: invalid entry size", {}, off + 8);
      const uint64_t data_length = _to_le(h.data_length);
      const uint64_t data_offset = off + sizeof(header) + name_length;
      const uint64_t crc_offset = data_offset + data_length;
      entries[i].name.resize(name_length);
      entries[i].entry_offset = off;
      entries[i].data_offset = data_offset;
      entries[i].data_length = data_length;
      if (auto res = fh->read_exact(entries[i].name.data(), name_length); !res) return unexpected_error(res.error());
      if (auto res = fh->seek(data_length, seek_whence::current); !res) return unexpected_error(res.error());
      if (auto res = fh->read_trivial<uint32_t>(); !res) return unexpected_error(res.error());
      else entries[i].crc32 = _to_le(*res);
      const bool is_last = (i == entry_count - 1);
      if (auto res = fh->tell(); !res) return unexpected_error(res.error());
      else if (const auto cur = *res; (is_last && cur != footer_offset) || (!is_last && cur != offsets[i + 1]))
        return unexpected_error(e, "archive: invalid entry size", {}, crc_offset);
    }
    return handle(std::move(*fh), std::move(entries), {offsets[0], footer_offset}, mode);
  }

  /// returns whether the archive is open
  bool is_open() const noexcept { return _fh.is_open(); }

  /// returns the path of the archive file
  const std::filesystem::path& path() const noexcept { return _fh.path(); }

  /// returns the open mode of the archive
  open_mode mode() const noexcept { return _mode; }

  /// returns the entries in the archive
  const std::vector<entry>& entries() const noexcept { return _entries; }

  /// closes the archive
  std::expected<void, error_trace> close() {
    if (!is_open()) return unexpected_error(errors::not_initialized, "not initialized");
    if (auto res = flush(); !res) return unexpected_error(res.error());
    if (auto res = _fh.close(); !res) return unexpected_error(res.error());
    _entries.clear();
    return {};
  }

  handle() noexcept = default;
  ~handle() {
    if (is_open()) close();
  }
  handle(const handle&) = delete;
  handle& operator=(const handle&) = delete;
  handle(handle&&) = default;
  handle& operator=(handle&&) = default;

  /// reads the data of an entry by index
  std::expected<std::vector<std::byte>, error_trace> read(size_t index) {
    if (index >= _entries.size()) return unexpected_error(errors::invalid_argument, "index out of range");
    const auto& e = _entries[index];
    if (auto res = _fh.seek(int64_t(e.data_offset)); !res) return unexpected_error(res.error());
    std::vector<std::byte> data(e.data_length);
    if (auto res = _fh.read_exact(data.data(), e.data_length); !res) return unexpected_error(res.error());
    return data;
  }

  /// reads the data of an entry by name
  std::expected<std::vector<std::byte>, error_trace> read(stringable<char> auto&& name) {
    const std::string_view sv(name);
    for (size_t i = 0; i < _entries.size(); ++i)
      if (_entries[i].name == sv) return read(i);
    return unexpected_error(errors::invalid_argument, "entry not found");
  }

  /// verifies the CRC32 of an entry by index
  std::expected<bool, error_trace> verify(size_t index) {
    if (index >= _entries.size()) return unexpected_error(errors::invalid_argument, "index out of range");
    const auto& e = _entries[index];
    if (auto res = _fh.seek(int64_t(e.data_offset)); !res) return unexpected_error(res.error());
    uint32_t crc = 0xFFFFFFFF;
    const size_t buffer_size = 4096;
    std::vector<std::byte> buffer(buffer_size);
    for (size_t remaining = e.data_length; remaining > 0;) {
      const size_t to_read = yw::min(buffer_size, remaining);
      if (auto res = _fh.read_exact(buffer.data(), to_read); !res) return unexpected_error(res.error());
      for (size_t i = 0; i < to_read; ++i) {
        crc ^= static_cast<uint8_t>(buffer[i]);
        for (int j = 0; j < 8; ++j) crc = bool(crc & 1) ? (crc >> 1) ^ 0xEDB88320 : (crc >> 1);
      }
      remaining -= to_read;
    }
    return crc ^ 0xFFFFFFFF == e.crc32;
  }

  /// verifies the CRC32 of an entry by name
  std::expected<bool, error_trace> verify(stringable<char> auto&& name) {
    const std::string_view sv(name);
    for (size_t i = 0; i < _entries.size(); ++i)
      if (_entries[i].name == sv) return verify(i);
    return unexpected_error(errors::invalid_argument, "entry not found");
  }

  /// flushes the archive footer
  std::expected<void, error_trace> flush() {
    if (_mode != open_mode::create_always && _mode != open_mode::create_new && _mode != open_mode::update_existing &&
        _mode != open_mode::update_or_create)
      return unexpected_error(errors::invalid_operation, "archive not opened in write mode");
    if (auto res = _fh.seek(int64_t(_footer_offset)); !res) return unexpected_error(res.error());
    footer f{_to_le(footer_magic), _to_le(static_cast<uint32_t>(_entries.size()))};
    if (auto res = _fh.write_trivial(f); !res) return unexpected_error(res.error());
    std::vector<uint64_t> offsets(_entries.size() + 1);
    for (size_t i = 0; i < _entries.size(); ++i) offsets[i] = _to_le(_entries[i].entry_offset);
    offsets.back() = _to_le(_footer_offset);
    if (auto res = _fh.write_exact(offsets.data(), offsets.size() * sizeof(uint64_t)); !res)
      return unexpected_error(res.error());
    if (auto res = _fh.truncate_to_current(); !res) return unexpected_error(res.error());
    return {};
  }

  /// removes the last `n` entries from the archive
  std::expected<void, error_trace> remove(size_t n) {
    if (n == npos) n = _entries.size();
    else if (n > _entries.size()) return unexpected_error(errors::invalid_argument, "n exceeds entry count");
    if (n == 0) {
      _footer_offset = _entry_offset;
      _entries.clear();
      return {};
    }
    _footer_offset = _entries[_entries.size() - n].entry_offset;
    _entries.resize(_entries.size() - n);
    return {};
  }

  /// appends a new entry to the archive
  std::expected<void, error_trace> append(stringable<char> auto&& name, const void* data, size_t data_length) {
    if (!data && data_length) return unexpected_error(errors::invalid_argument, "null data pointer");
    const std::string_view sv(name);
    if (sv.size() == 0 || sv.size() > max_name_size)
      return unexpected_error(errors::invalid_argument, "archive: invalid entry name length");
    if (_mode != open_mode::create_always && _mode != open_mode::create_new && _mode != open_mode::update_existing &&
        _mode != open_mode::update_or_create)
      return unexpected_error(errors::invalid_operation, "archive not opened in write mode");
    if (auto res = _fh.seek(int64_t(_footer_offset)); !res) return unexpected_error(res.error());
    header h;
    h.magic = _to_le(entry_magic);
    h.name_length = _to_le(static_cast<uint32_t>(sv.size()));
    h.data_length = _to_le(static_cast<uint64_t>(data_length));
    if (auto res = _fh.write_trivial(h); !res) return unexpected_error(res.error());
    if (auto res = _fh.write_exact(sv.data(), sv.size()); !res) return unexpected_error(res.error());
    if (data_length > 0) {
      if (auto res = _fh.write_exact(data, data_length); !res) return unexpected_error(res.error());
    }
    uint32_t crc = 0xFFFFFFFF;
    const std::byte* p = static_cast<const std::byte*>(data);
    for (size_t i = 0; i < data_length; ++i) {
      crc ^= static_cast<uint8_t>(p[i]);
      for (int j = 0; j < 8; ++j) crc = bool(crc & 1) ? (crc >> 1) ^ 0xEDB88320 : (crc >> 1);
    }
    crc ^= 0xFFFFFFFF;
    if (auto res = _fh.write_trivial<uint32_t>(_to_le(crc)); !res) return unexpected_error(res.error());
    const auto data_offset = _footer_offset + sizeof(header) + sv.size();
    _entries.emplace_back(std::string(sv), _footer_offset, data_offset, data_length, crc);
    _footer_offset = data_offset + data_length + sizeof(uint32_t);
    return {};
  }
};

/// opens an archive file
inline std::expected<handle, error_trace> open(const std::filesystem::path& path, open_mode mode) {
  if (auto res = handle::create(path, mode); !res) return unexpected_error(res.error());
  else return std::move(*res);
}

/// packs files in `src_path` into an archive file `dst_path`.
inline std::expected<void, error_trace> pack(const std::filesystem::path& src_path,
  const std::filesystem::path& dst_path, open_mode mode = open_mode::create_always) {
  if (!std::filesystem::is_directory(src_path))
    return unexpected_error(errors::invalid_argument, "source path is not a directory");
  auto archive = handle::create(dst_path, mode);
  if (!archive) return unexpected_error(archive.error());
  for (const auto& entry : std::filesystem::recursive_directory_iterator(src_path)) {
    if (!entry.is_regular_file()) continue;
    auto fh = yw::open(entry.path(), open_mode::read_existing);
    if (!fh) return unexpected_error(fh.error());
    uint64_t file_size{};
    if (auto res = fh->seek(0, seek_whence::end); !res) return unexpected_error(res.error());
    if (auto res = fh->tell(); !res) return unexpected_error(res.error());
    else file_size = static_cast<uint64_t>(*res);
    if (auto res = fh->seek(0, seek_whence::begin); !res) return unexpected_error(res.error());
    std::vector<std::byte> data(file_size);
    if (auto res = fh->read_exact(data.data(), file_size); !res) return unexpected_error(res.error());
    const auto filename = unicode<char>(std::filesystem::relative(entry.path(), src_path).native());
    if (auto res = archive->append(filename, data.data(), file_size); !res) return unexpected_error(res.error());
  }
  if (auto res = archive->close(); !res) return unexpected_error(res.error());
  return {};
}

/// unpacks files in an archive file `src_path` into a directory `dst_path`.
inline std::expected<void, error_trace> extract(
  const std::filesystem::path& src_path, const std::filesystem::path& dst_path) {
  auto archive = handle::create(src_path, open_mode::read_existing);
  if (!archive) return unexpected_error(archive.error());
  for (const auto& e : archive->entries()) {
    auto data = archive->read(e.name);
    if (!data) return unexpected_error(data.error());
    const auto out_path = dst_path / unicode<char>(e.name);
    std::filesystem::create_directories(out_path.parent_path());
    auto fh = yw::open(out_path, open_mode::create_always);
    if (!fh) return unexpected_error(fh.error());
    if (auto res = fh->write_exact(data->data(), data->size()); !res) return unexpected_error(res.error());
    if (auto res = fh->close(); !res) return unexpected_error(res.error());
  }
  if (auto res = archive->close(); !res) return unexpected_error(res.error());
  return {};
}
} // namespace yw::archive
