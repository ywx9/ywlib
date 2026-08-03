#pragma once
#include <yw/file_handle.h>

/*
# .ywa file

## Purpose

Simplest uncompressed archive

## Rules

- All numeric fields are in little-endian
- No padding between entries or between entries and footer is allowed
- The implementation guarantees to work even if there is data before the start of an entry
- Entry names are UTF-8 strings less than 2048 bytes
- The order of offsets in the footer corresponds to the order of entries
- CRC32 is calculated for each entry's data according to IEEE 802.3
  - (poly=0x04C11DB7, init=0xFFFFFFFF, refin=true, refout=true, xorout=0xFFFFFFFF)
  - "123456789" => 0xCBF43926

## Structure

- Entry[N]
  - Magic number(4) == YWAE; (0x45415759)
  - File name length(4)
  - Data length(8)
  - File name
  - Data
  - CRC32(4)
- Footer
  - Magic number(4) == YWAF; (0x46415759)
  - File count(4) == N
  - Offsets of each entry from the beginning of the file(8 x N)
  - Offset of the footer from the beginning of the file(8)
*/

namespace yw::archive {

inline constexpr uint32_t entry_magic = 0x45415759;  // 'YWAE'
inline constexpr uint32_t footer_magic = 0x46415759; // 'YWAF'
inline constexpr uint32_t max_name_size = 2048;

inline constexpr bool _is_write_mode(const file::open_mode mode) noexcept {
  return mode == file::open_mode::create_always || mode == file::open_mode::create_new ||
         mode == file::open_mode::update_existing || mode == file::open_mode::update_or_create;
}

inline constexpr uint32_t _crc32_update(uint32_t crc, const std::byte* data, const size_t size) noexcept {
  for (size_t i = 0; i < size; ++i) {
    crc ^= static_cast<uint8_t>(data[i]);
    for (int j = 0; j < 8; ++j) crc = bool(crc & 1) ? (crc >> 1) ^ 0xEDB88320 : (crc >> 1);
  }
  return crc;
}

struct entry {
  string<char> name;
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

class handle : public handle_base {
public:
  struct slot : handle_base::slot {
    file::handle file_handle;
    std::vector<entry> entries;
    file::open_mode mode = file::open_mode::unknown;
    uint64_t entry_offset = 0;
    uint64_t footer_offset = 0;

    std::expected<void, error> initialize(stringable auto&& Path, file::open_mode Mode) {
      const auto fh_mode = Mode == file::open_mode::append ? file::open_mode::update_or_create : Mode;
      if (auto fh = file::handle::create(static_cast<decltype(Path)&&>(Path), fh_mode)) file_handle = std::move(*fh);
      else return fh.error().relay();
      mode = Mode;
      if (fh_mode == file::open_mode::create_always || fh_mode == file::open_mode::create_new) return {};
      if (auto res = file_handle.seek(0, file::seek_whence::end); !res) return res.error().relay();
      const auto fsize = static_cast<uint64_t>(file_handle.tell());
      if (fsize == 0) {
        if (fh_mode == file::open_mode::update_or_create) return {};
        else return std::unexpected(error(errors::invalid_file_format, "non-archive file"));
      }
      if (auto res = file_handle.seek(-8, file::seek_whence::end); !res) return res.error().relay();
      if (auto res = file_handle.read_trivial<uint64_t>(footer_offset); !res) return res.error().relay();
      if (footer_offset + sizeof(footer) + sizeof(uint64_t) > fsize)
        return std::unexpected(error(errors::invalid_file_format, "invalid footer offset"));
      footer f{};
      if (auto res = file_handle.seek(static_cast<int64_t>(footer_offset)); !res) return res.error().relay();
      if (auto res = file_handle.read_trivial(f); !res) return res.error().relay();
      if (f.magic != footer_magic) return std::unexpected(error(errors::invalid_file_format, "invalid footer magic"));
      const auto entry_count = f.entry_count;
      const auto footer_size =
        sizeof(footer) + static_cast<uint64_t>(entry_count) * sizeof(uint64_t) + sizeof(uint64_t);
      if (footer_offset + footer_size != fsize)
        return std::unexpected(error(errors::invalid_file_format, "invalid entry count"));
      if (entry_count == 0) {
        entry_offset = footer_offset;
        return {};
      }
      std::vector<uint64_t> offsets(entry_count);
      if (auto res = file_handle.read_exact(offsets.data(), offsets.size() * sizeof(uint64_t)); !res)
        return res.error().relay();
      entries.resize(entry_count);
      entry_offset = offsets.front();
      for (uint32_t i = 0; i < entry_count; ++i) {
        const auto off = offsets[i];
        if (off >= footer_offset) return std::unexpected(error(errors::invalid_file_format, "invalid entry offset"));
        if (auto res = file_handle.seek(static_cast<int64_t>(off)); !res) return res.error().relay();
        header h{};
        if (auto res = file_handle.read_trivial(h); !res) return res.error().relay();
        if (h.magic != entry_magic) return std::unexpected(error(errors::invalid_file_format, "invalid entry magic"));
        const auto name_length = h.name_length;
        if (name_length == 0 || name_length > max_name_size)
          return std::unexpected(error(errors::invalid_file_format, "invalid name length"));
        if (off + sizeof(header) + name_length > footer_offset)
          return std::unexpected(error(errors::invalid_file_format, "invalid entry size"));
        const auto data_length = h.data_length;
        const auto data_offset = off + sizeof(header) + name_length;
        const auto crc_offset = data_offset + data_length;
        auto& e = entries[i];
        e.name.resize(name_length);
        e.entry_offset = off;
        e.data_offset = data_offset;
        e.data_length = data_length;
        if (auto res = file_handle.read_exact(e.name.data(), name_length); !res) return res.error().relay();
        if (auto res = file_handle.seek(static_cast<int64_t>(data_length), file::seek_whence::current); !res)
          return res.error().relay();
        if (auto res = file_handle.read_trivial<uint32_t>(e.crc32); !res) return res.error().relay();
        const bool is_last = i + 1 == entry_count;
        const auto next = is_last ? footer_offset : offsets[i + 1];
        if (crc_offset + sizeof(uint32_t) != next)
          return std::unexpected(error(errors::invalid_file_format, "invalid entry size"));
      }
      return {};
    }
  };

  explicit operator bool() const noexcept {
    const auto sp = get_slot(this);
    return sp && static_cast<bool>(sp->file_handle);
  }

  handle() noexcept = default;

  static std::expected<handle, error> create(stringable auto&& Path, file::open_mode m) {
    const auto sp = make_slot<handle>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    if (auto res = sp->initialize(static_cast<decltype(Path)&&>(Path), m); !res) {
      erase_slot(sp->id);
      return res.error().relay();
    } else return make_handle<handle>(sp->id);
  }

  /// creates handle of archive file
  handle(stringable auto&& Path, file::open_mode m, const source_line& sl = here()) {
    if (auto res = create(static_cast<decltype(Path)&&>(Path), m)) *this = std::move(*res);
    else res.error().go_off(sl);
  }


  bool is_open() const noexcept {
    const auto sp = get_slot(this);
    return sp && sp->file_handle.is_open();
  }

  const string<file::path_char>& path() const {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->file_handle.path();
  }

  file::open_mode mode() const {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return file::open_mode::unknown;
    } else return sp->mode;
  }

  const std::vector<entry>& entries() const {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->entries;
  }

  std::expected<void, error> close() {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    if (!sp->file_handle.is_open()) return std::unexpected(error(errors::not_initialized));
    if (_is_write_mode(sp->mode))
      if (auto res = flush(); !res) return res.error().relay();
    if (auto res = sp->file_handle.close(); !res) return res.error().relay();
    sp->entries.clear();
    return {};
  }

  std::expected<void*, error> read(size_t Index, void* Out) {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    if (Index >= sp->entries.size()) return std::unexpected(error(errors::invalid_argument, "index out of range"));
    const auto& e = sp->entries[Index];
    auto& fh = sp->file_handle;
    if (auto res = fh.seek(static_cast<int64_t>(e.data_offset)); !res) return res.error().relay();
    if (auto res = fh.read_exact(Out, static_cast<size_t>(e.data_length)); !res) return res.error().relay();
    else return static_cast<void*>(reinterpret_cast<std::byte*>(Out) + e.data_length);
  }

  std::vector<std::byte> read(size_t index) {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out(); // warning
      return {};
    }
    if (index >= sp->entries.size()) {
      error(errors::invalid_argument, "index out of range").fizzle_out(); // warning
      return {};
    }
    const auto& e = sp->entries[index];
    if (auto res = sp->file_handle.seek(static_cast<int64_t>(e.data_offset)); !res) {
      res.error().add_footprint().fizzle_out(); // warning
      return {};
    }
    std::vector<std::byte> data(static_cast<size_t>(e.data_length));
    if (auto res = sp->file_handle.read_exact(data.data(), data.size()); !res) {
      res.error().add_footprint().fizzle_out(); // warning
      return {};
    }
    return data;
  }

  std::vector<std::byte> read(stringable<char> auto&& name) {
    const auto sv = string_view<char>(name);
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out(); // warning
      return {};
    }
    for (size_t i = 0; i < sp->entries.size(); ++i)
      if (sp->entries[i].name.view() == sv) return read(i);
    error(errors::invalid_argument, "entry not found").fizzle_out(); // warning
    return {};
  }

  bool verify(size_t index) {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out(); // warning
      return false;
    }
    if (index >= sp->entries.size()) {
      error(errors::invalid_argument, "index out of range").fizzle_out(); // warning
      return false;
    }
    const auto& e = sp->entries[index];
    if (auto res = sp->file_handle.seek(static_cast<int64_t>(e.data_offset)); !res) {
      res.error().add_footprint().fizzle_out(); // warning
      return false;
    }
    uint32_t crc = 0xFFFFFFFF;
    constexpr size_t buffer_size = 4096;
    std::vector<std::byte> buffer(buffer_size);
    for (uint64_t remaining = e.data_length; remaining > 0;) {
      const auto to_read = yw::min(static_cast<uint64_t>(buffer_size), remaining);
      if (auto res = sp->file_handle.read_exact(buffer.data(), static_cast<size_t>(to_read)); !res) {
        res.error().add_footprint().fizzle_out(); // warning
        return false;
      }
      crc = _crc32_update(crc, buffer.data(), static_cast<size_t>(to_read));
      remaining -= to_read;
    }
    return (crc ^ 0xFFFFFFFF) == e.crc32;
  }

  bool verify(stringable<char> auto&& name) {
    const auto sv = string_view<char>(name);
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out(); // warning
      return false;
    }
    for (size_t i = 0; i < sp->entries.size(); ++i)
      if (sp->entries[i].name.view() == sv) return verify(i);
    error(errors::invalid_argument, "entry not found").fizzle_out(); // warning
    return false;
  }

  std::expected<void, error> flush() {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    if (!_is_write_mode(sp->mode))
      return std::unexpected(error(errors::invalid_operation, "archive not opened in write mode"));
    if (auto res = sp->file_handle.seek(static_cast<int64_t>(sp->footer_offset)); !res) return res.error().relay();
    const footer f{footer_magic, static_cast<uint32_t>(sp->entries.size())};
    if (auto res = sp->file_handle.write_exact(&f, sizeof(f)); !res) return res.error().relay();
    std::vector<uint64_t> offsets(sp->entries.size() + 1);
    for (size_t i = 0; i < sp->entries.size(); ++i) offsets[i] = sp->entries[i].entry_offset;
    offsets.back() = sp->footer_offset;
    if (auto res = sp->file_handle.write_exact(offsets.data(), offsets.size() * sizeof(uint64_t)); !res)
      return res.error().relay();
    if (auto res = sp->file_handle.truncate_to_current(); !res) return res.error().relay();
    return {};
  }

  std::expected<void, error> remove(size_t n = npos) {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    if (!_is_write_mode(sp->mode))
      return std::unexpected(error(errors::invalid_operation, "archive not opened in write mode"));
    if (n == npos) n = sp->entries.size();
    else if (n > sp->entries.size()) return std::unexpected(error(errors::invalid_argument, "n exceeds entry count"));
    if (n == 0) {
      sp->footer_offset = sp->entry_offset;
      sp->entries.clear();
      return {};
    }
    sp->footer_offset = sp->entries[sp->entries.size() - n].entry_offset;
    sp->entries.resize(sp->entries.size() - n);
    return {};
  }

  std::expected<void, error> append(stringable<char> auto&& name, const void* data, size_t data_length) {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    if (!data && data_length) return std::unexpected(error(errors::invalid_argument, "null data pointer"));
    const auto sv = string_view<char>(name);
    if (sv.empty() || sv.size() > max_name_size)
      return std::unexpected(error(errors::invalid_argument, "archive: invalid entry name length"));
    if (!_is_write_mode(sp->mode))
      return std::unexpected(error(errors::invalid_operation, "archive not opened in write mode"));
    if (auto res = sp->file_handle.seek(static_cast<int64_t>(sp->footer_offset)); !res) return res.error().relay();

    const header h{entry_magic, static_cast<uint32_t>(sv.size()), static_cast<uint64_t>(data_length)};
    if (auto res = sp->file_handle.write_exact(&h, sizeof(h)); !res) return res.error().relay();
    if (auto res = sp->file_handle.write_exact(sv.data(), sv.size()); !res) return res.error().relay();
    if (data_length > 0)
      if (auto res = sp->file_handle.write_exact(data, data_length); !res) return res.error().relay();

    uint32_t crc = 0xFFFFFFFF;
    if (data_length > 0) crc = _crc32_update(crc, static_cast<const std::byte*>(data), data_length);
    crc ^= 0xFFFFFFFF;
    if (auto res = sp->file_handle.write_exact(&crc, sizeof(crc)); !res) return res.error().relay();

    const auto new_entry_offset = sp->footer_offset;
    const auto data_offset = new_entry_offset + sizeof(header) + sv.size();
    if (sp->entries.empty()) sp->entry_offset = new_entry_offset;
    sp->entries.push_back(entry{string<char>(sv), new_entry_offset, data_offset, data_length, crc});
    sp->footer_offset = data_offset + data_length + sizeof(uint32_t);
    return {};
  }
};

inline handle open(stringable auto&& path, file::open_mode mode, const source_line& sl = here()) {
  if (auto res = handle::create(static_cast<decltype(path)&&>(path), mode)) return std::move(*res);
  else res.error().fizzle_out(sl);
  return {};
}

inline std::expected<void, error> pack(stringable auto&& src_path, stringable auto&& dst_path,
  file::open_mode mode = file::open_mode::create_always) {
  if (!file::is_directory(src_path))
    return std::unexpected(error(yw::errors::invalid_argument, "source path is not a directory"));
  auto archive = handle::create(static_cast<decltype(dst_path)&&>(dst_path), mode);
  if (!archive) return std::unexpected(error(yw::errors::operation_failed, "failed to open archive file"));
  for (const auto& item : file::list_files(src_path, true)) {
    if (!file::is_file(item)) continue;
    auto fh = file::handle::create(static_cast<decltype(item)&&>(item), file::open_mode::read_existing);
    if (!fh) return std::unexpected(error(yw::errors::operation_failed, "failed to open source file"));
    if (auto res = fh.seek(0, file::seek_whence::end); !res) return res.error().relay();
    const auto file_size = static_cast<uint64_t>(fh.tell());
    if (auto res = fh.seek(0, file::seek_whence::begin); !res) return res.error().relay();
    std::vector<std::byte> data(static_cast<size_t>(file_size));
    if (auto res = fh.read_exact(data.data(), data.size()); !res) return res.error().relay();
    const auto filename = unicode<char>(file::relative(item.path(), src_path).native());
    if (auto res = archive->append(filename, data.data(), data.size()); !res) return res.error().relay();
  }
  if (auto res = archive->close(); !res) return res.error().relay();
  return {};
}

inline std::expected<void, error> extract(stringable auto&& src_path, stringable auto&& dst_path) {
  auto archive = handle::create(static_cast<decltype(src_path)&&>(src_path), file::open_mode::read_existing);
  if (!archive) return std::unexpected(error(yw::errors::operation_failed, "failed to open archive file"));
  for (const auto& e : archive.entries()) {
    auto data = archive->read(e.name);
    if (data.size() != e.data_length)
      return std::unexpected(error(yw::errors::operation_failed, "failed to read entry"));
    const auto out_path = format<file::path_char>(dst_path, file::path_char('/'), e.name);
    const auto parent = file::parent(out_path);
    file::create_directories(parent);
    auto fh = file::handle::create(out_path, file::open_mode::create_always);
    if (!fh) return std::unexpected(error(yw::errors::operation_failed, "failed to open output file"));
    if (auto res = fh.write_exact(data.data(), data.size()); !res) return res.error().relay();
    if (auto res = fh.close(); !res) return res.error().relay();
  }
  if (auto res = archive->close(); !res) return res.error().relay();
  return {};
}
} // namespace yw::archive
