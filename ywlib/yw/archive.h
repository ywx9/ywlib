#pragma once

#include "yw/core.h"
#include "yw/file.h"

#include <vector>

/**
 * .ywaファイル
 * 用途: Simplestな非圧縮アーカイブ
 * 規則:
 * - リトルエンディアン
 * - エントリ-エントリ間、エントリ-フッタ間にパディングは入らない (エントリ直後がYWAであること)
 * - ファイル名はUTF-8エンコードされた文字列で2048バイトまで
 * 構造:
 * - (任意のデータがあってもよい)
 * - エントリ[N]
 *   - マジックナンバ(4) == YWAE; (0x45415759)
 *   - ファイル名の長さ(4)
 *   - データの長さ(8)
 *   - ファイル名
 *   - データ
 * - フッタ
 *   - マジックナンバ(4) == YWAF; (0x46415759)
 *   - ファイル数(4) == N
 *   - 各エントリのファイル先頭からのオフセット(8 x N)
 *   - フッタのファイル先頭からのオフセット(8)
 */

namespace yw {

inline const struct {
  static_assert(std::endian::native == std::endian::little, "archive: only little-endian is supported");
  const std::uint32_t entry_magic = 0x45415759;  // 'YWAE'
  const std::uint32_t footer_magic = 0x46415759; // 'YWAF'
  const std::uint32_t max_name_size = 2048;

  struct entry_header {
    std::uint32_t magic;
    std::uint32_t name_length;
    std::uint64_t data_length;
  };
  static_assert(sizeof(entry_header) == 16);

  struct footer_header {
    std::uint32_t magic;
    std::uint32_t file_count;
  };
  static_assert(sizeof(footer_header) == 8);

  struct entry {
    std::string name;
    std::uint64_t entry_offset;
    std::uint64_t data_offset;
    std::uint64_t data_length;
  };

  /// archive writer/reader
  class handler {
    file_handler _fh;
    std::vector<entry> _entries;
    explicit handler(file_handler&& fh) : _fh(std::move(fh)) {}

  public:
    handler() noexcept = default;
    handler(const handler&) = delete;
    handler& operator=(const handler&) = delete;
    handler(handler&&) noexcept = default;
    handler& operator=(handler&&) noexcept = default;
    ~handler() = default;

    /// creates and opens an archive handler
    /// \return opened archive handler or error string
    static std::expected<handler, error> create(const std::filesystem::path& p, open_mode mode) {
      if (auto fexp = yw::open(p, mode); !fexp) return std::unexpected(fexp.error());
      else return handler(std::move(fexp.value()));
    }

    bool is_open() const noexcept { return _fh.is_open(); }
    const std::filesystem::path& path() const noexcept { return _fh.path(); }
    const std::vector<entry>& entries() const noexcept { return _entries; }

    std::expected<std::vector<std::byte>, error> extract(stringable<char> auto&& name) {
      if (!_fh.is_open()) return std::unexpected("file not open");
      auto sv = std::string_view(name);
      for (const auto& e : _entries)
        if (e.name == sv) {
          if (!_fh.seek(e.data_offset)) return std::unexpected("failed to seek to data offset");
          std::vector<std::byte> data(e.data_length);
          if (auto rex = _fh.read_exact(data.data(), e.data_length); !rex) return std::unexpected(rex.error());
          return data;
        }
      return std::unexpected("entry not found");
    }
  };

  class reader {
    file_handler _fh;
    std::vector<entry> _metas;
    explicit reader(const std::filesystem::path& p) {
      if (auto fexp = yw::open(p, open_mode::read_existing); !fexp)
        throw std::runtime_error("archive::reader: failed to open file: " + fexp.error());
    }

  public:
    static reader create(const std::filesystem::path& path) : _fh(path) {
      const auto file_size = _fh.file_size();
      if (file_size < sizeof(footer_header))
        throw std::runtime_error("archive::reader: file too small to be a valid archive");

      // Read footer
      _reader.seek(file_size - sizeof(std::uint64_t));
      std::uint64_t footer_offset;
      _reader.read_exact(&footer_offset, sizeof(footer_offset));
      if (footer_offset + sizeof(footer_header) > static_cast<std::uint64_t>(file_size))
        throw std::runtime_error("archive::reader: invalid footer offset");

      _reader.seek(footer_offset);
      footer_header footer;
      _reader.read_exact(&footer, sizeof(footer));
      if (footer.magic != footer_magic) throw std::runtime_error("archive::reader: invalid footer magic");
      if (footer.file_count == 0) throw std::runtime_error("archive::reader: archive contains no entries");

      // Read entry offsets
      std::vector<std::uint64_t> entry_offsets(footer.file_count);
      _reader.read_exact(entry_offsets.data(), entry_offsets.size() * sizeof(std::uint64_t));

      // Read entries
      _metas.reserve(footer.file_count);
      for (std::uint32_t i = 0; i < footer.file_count; ++i) {
        _reader.seek(entry_offsets[i]);
        entry_header eh;
        _reader.read_exact(&eh, sizeof(eh));

        if (eh.magic != entry_magic) throw std::runtime_error("archive::reader: invalid entry magic");
        if (eh.name_length == 0 || eh.name_length > max_name_size)
          throw std::runtime_error("archive::reader: invalid entry name length");

        std::string name(eh.name_length, '\0');
        _reader.read_exact(name.data(), eh.name_length);

        const std::uint64_t data_offset = entry_offsets[i] + sizeof(eh) + eh.name_length;
        _metas.push_back(entry{name, entry_offsets[i], data_offset, eh.data_length});
      }
    }

    bool is_open() const noexcept { return _reader.is_open(); }

    const std::vector<entry>& entries() const noexcept { return _metas; }

    std::vector<std::byte> extract(stringable<char> auto&& name) {
      if (!_reader.is_open()) throw std::runtime_error("archive::reader: reader is not open");
      auto sv = std::string_view(name);
      for (const auto& e : _metas) {
        if (e.name == sv) {
          _reader.seek(e.data_offset);
          std::vector<std::byte> data(e.data_length);
          _reader.read_exact(data.data(), e.data_length);
          return data;
        }
      }
      throw std::runtime_error("archive::reader: entry not found");
    }

    void extract_to(stringable<char> auto&& name, yw::writer& out) {
      if (!_reader.is_open()) throw std::runtime_error("archive::reader: reader is not open");
      auto sv = std::string_view(name);
      for (const auto& e : _metas) {
        if (e.name == sv) {
          _reader.seek(e.data_offset);
          const std::size_t buffer_size = 4096;
          std::vector<std::byte> buffer(buffer_size);
          std::uint64_t remaining = e.data_length;
          while (remaining > 0) {
            const std::size_t to_read = static_cast<std::size_t>(std::min<std::uint64_t>(buffer_size, remaining));
            _reader.read_exact(buffer.data(), to_read);
            out.write_exact(buffer.data(), to_read);
            remaining -= to_read;
          }
          return;
        }
      }
      throw std::runtime_error("archive::reader: entry not found");
    }
  };

} archive;

} // namespace yw

namespace yw_test {

class archive {
public:
  static_assert(std::endian::native == std::endian::little, "archive: only little-endian is supported");
  static constexpr std::uint32_t entry_magic = 0x45415759;  // 'YWAE'
  static constexpr std::uint32_t footer_magic = 0x46415759; // 'YWAF'
  static constexpr std::uint32_t max_name_size = 2048;

  struct entry_header {
    std::uint32_t magic;
    std::uint32_t name_length;
    std::uint64_t data_length;
  };
  static_assert(sizeof(entry_header) == 16);

  struct footer_header {
    std::uint32_t magic;
    std::uint32_t file_count;
  };
  static_assert(sizeof(footer_header) == 8);

  struct entry {
    std::string name;
    std::uint64_t entry_offset;
    std::uint64_t data_offset;
    std::uint64_t data_length;
  };

  class reader {
    yw::reader _reader;
    std::vector<entry> _metas;

  public:
    explicit reader(const std::filesystem::path& path) : _reader(path) {
      const auto file_size = _reader.size();
      if (file_size < sizeof(footer_header))
        throw std::runtime_error("archive::reader: file too small to be a valid archive");

      // Read footer
      _reader.seek(file_size - sizeof(std::uint64_t));
      std::uint64_t footer_offset;
      _reader.read_exact(&footer_offset, sizeof(footer_offset));
      if (footer_offset + sizeof(footer_header) > static_cast<std::uint64_t>(file_size))
        throw std::runtime_error("archive::reader: invalid footer offset");

      _reader.seek(footer_offset);
      footer_header footer;
      _reader.read_exact(&footer, sizeof(footer));
      if (footer.magic != footer_magic) throw std::runtime_error("archive::reader: invalid footer magic");
      if (footer.file_count == 0) throw std::runtime_error("archive::reader: archive contains no entries");

      // Read entry offsets
      std::vector<std::uint64_t> entry_offsets(footer.file_count);
      _reader.read_exact(entry_offsets.data(), entry_offsets.size() * sizeof(std::uint64_t));

      // Read entries
      _metas.reserve(footer.file_count);
      for (std::uint32_t i = 0; i < footer.file_count; ++i) {
        _reader.seek(entry_offsets[i]);
        entry_header eh;
        _reader.read_exact(&eh, sizeof(eh));

        if (eh.magic != entry_magic) throw std::runtime_error("archive::reader: invalid entry magic");
        if (eh.name_length == 0 || eh.name_length > max_name_size)
          throw std::runtime_error("archive::reader: invalid entry name length");

        std::string name(eh.name_length, '\0');
        _reader.read_exact(name.data(), eh.name_length);

        const std::uint64_t data_offset = entry_offsets[i] + sizeof(eh) + eh.name_length;
        _metas.push_back(entry{name, entry_offsets[i], data_offset, eh.data_length});
      }
    }

    bool is_open() const noexcept { return _reader.is_open(); }

    const std::vector<entry>& entries() const noexcept { return _metas; }

    std::vector<std::byte> extract(stringable<char> auto&& name) {
      if (!_reader.is_open()) throw std::runtime_error("archive::reader: reader is not open");
      auto sv = std::string_view(name);
      for (const auto& e : _metas) {
        if (e.name == sv) {
          _reader.seek(e.data_offset);
          std::vector<std::byte> data(e.data_length);
          _reader.read_exact(data.data(), e.data_length);
          return data;
        }
      }
      throw std::runtime_error("archive::reader: entry not found");
    }

    void extract_to(stringable<char> auto&& name, yw::writer& out) {
      if (!_reader.is_open()) throw std::runtime_error("archive::reader: reader is not open");
      auto sv = std::string_view(name);
      for (const auto& e : _metas) {
        if (e.name == sv) {
          _reader.seek(e.data_offset);
          const std::size_t buffer_size = 4096;
          std::vector<std::byte> buffer(buffer_size);
          std::uint64_t remaining = e.data_length;
          while (remaining > 0) {
            const std::size_t to_read = static_cast<std::size_t>(std::min<std::uint64_t>(buffer_size, remaining));
            _reader.read_exact(buffer.data(), to_read);
            out.write_exact(buffer.data(), to_read);
            remaining -= to_read;
          }
          return;
        }
      }
      throw std::runtime_error("archive::reader: entry not found");
    }
  };

  class writer {
    yw::writer _writer;
    std::vector<entry> _metas;
    uint64_t _base_offset = 0;

  public:
    writer() noexcept = default;

    explicit writer(const std::filesystem::path& path) : _writer(path, yw::writer::open_mode::update_or_create) {
      if (!_writer.is_open()) throw std::runtime_error("archive::writer: failed to open file for writing");
      _writer.seek(0, yw::writer::whence::end);
      _base_offset = _writer.tell();
    }

    ~writer() {
      if (_writer.is_open()) {
        try {
          finalize();
        } catch (...) {}
      }
    }

    bool is_open() const noexcept { return _writer.is_open(); }

    void add_entry(stringable<char> auto&& name, const void* data, std::uint64_t size) {
      if (!_writer.is_open()) throw std::runtime_error("archive::writer: writer is not open");
      auto sv = std::string_view(name);
      if (sv.size() == 0 || sv.size() > max_name_size)
        throw std::runtime_error("archive::writer: invalid entry name length");

      const std::uint64_t entry_offset = _writer.tell();
      entry_header eh{entry_magic, static_cast<std::uint32_t>(sv.size()), size};
      _writer.write_exact(&eh, sizeof(eh));
      _writer.write_exact(sv.data(), sv.size());
      _writer.write_exact(data, size);

      const std::uint64_t data_offset = entry_offset + sizeof(eh) + sv.size();
      _metas.push_back(entry{std::string(sv), entry_offset, data_offset, size});
    }

    /// removes the last 'count' entries added
    void remove_entry(std::uint64_t count) {
      if (!_writer.is_open()) throw std::runtime_error("archive::writer: writer is not open");
      if (count > _metas.size())
        throw std::runtime_error("archive::writer: remove_entry count exceeds number of entries");
      _metas.resize(_metas.size() - count);
      if (!_metas.empty()) {
        const auto& last_entry = _metas.back();
        _writer.seek(last_entry.data_offset + last_entry.data_length);
      } else _writer.seek(_base_offset);
    }

    /// finalizes the archive file by writing the footer and closing the writer
    void finalize() {
      if (!_writer.is_open()) throw std::runtime_error("archive::writer: writer is not open");
      const std::uint64_t footer_offset = _writer.tell();
      footer_header footer{footer_magic, static_cast<std::uint32_t>(_metas.size())};
      _writer.write_exact(&footer, sizeof(footer));
      for (const auto& e : _metas) _writer.write_exact(&e.entry_offset, sizeof(e.entry_offset));
      _writer.write_exact(&footer_offset, sizeof(footer_offset));
      _writer.close_at_current();
      _metas.clear();
    }
  };

  /// archives all files in 'source_dir' into 'archive_path'
  archive(const std::filesystem::path& source_dir, const std::filesystem::path& archive_path) {
    writer arch_writer(archive_path);
    if (!arch_writer.is_open())
      throw std::runtime_error("archive::create_archive: failed to open archive file for writing");

    for (const auto& entry : std::filesystem::recursive_directory_iterator(source_dir)) {
      if (entry.is_regular_file()) {
        const auto relative_path = std::filesystem::relative(entry.path(), source_dir).string();
        auto reader = yw::reader(entry.path());
        if (!reader.is_open())
          throw std::runtime_error("archive::create_archive: failed to open source file for reading");
        const auto file_size = reader.size();
        std::vector<std::byte> file_data(file_size);
        reader.read_exact(file_data.data(), file_size);
        arch_writer.add_entry(relative_path, file_data.data(), file_size);
      }
    }

    arch_writer.finalize();
  }

  /// extracts all entries from 'archive_path' into 'output_dir'
  static void extract_all(const std::filesystem::path& archive_path, const std::filesystem::path& output_dir) {
    reader arch_reader(archive_path);
    if (!arch_reader.is_open())
      throw std::runtime_error("archive::extract_all: failed to open archive file for reading");

    for (const auto& e : arch_reader.entries()) {
      const auto output_path = output_dir / e.name;
      std::filesystem::create_directories(output_path.parent_path());
      yw::writer writer(output_path, yw::writer::open_mode::truncate);
      if (!writer.is_open()) throw std::runtime_error("archive::extract_all: failed to open output file for writing");
      arch_reader.extract_to(e.name, writer);
    }
  }
};

} // namespace yw_test
