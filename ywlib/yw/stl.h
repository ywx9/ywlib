#pragma once
#include <yw/file.h>
#include <yw/math.h>
#include <yw/vector.h>

namespace yw {

class stl {
public:
#pragma pack(push, 1)
  struct triangle {
    float3 normal{};
    float3 v0{};
    float3 v1{};
    float3 v2{};
    uint16_t attribute_byte_count = 0;

    constexpr triangle& set(float3 V0, float3 V1, float3 V2) noexcept {
      v0 = V0;
      v1 = V1;
      v2 = V2;
      normal = cross(v1 - v0, v2 - v0);
      const auto len = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
      if (len > 0.0f) normal /= len;
      return *this;
    }
  };
#pragma pack(pop)
  static_assert(sizeof(triangle) == 50);

private:
  std::array<char, 80> header{};
  std::vector<triangle> triangles;

public:
  stl() noexcept = default;

  explicit stl(size_t TriangleCount, std::array<char, 80> Header = {}) : header(Header), triangles(TriangleCount) {}

  explicit stl(const file::path& Path, const source_line& sl = here()) {
    if (auto res = create(Path)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<stl, error> create(const file::path& Path) {
    if (!file::exists(Path)) return std::unexpected(error(errors::operation_failed, "file not found"));
    if (auto f = file_handle::create(Path, open_mode::read_existing)) {
      const auto fsize = f->file_size();
      if (fsize < 84) return std::unexpected(error(errors::invalid_file_format, "binary STL is too small"));
      stl m;
      if (auto res = f->read_exact(m.header.data(), m.header.size()); !res) return res.error().relay();
      uint32_t count{};
      if (auto res = f->read_trivial(count); !res) return res.error().relay();
      if (count == 0) return m;
      m.triangles.resize(count);
      if (auto res = f->read_exact(m.triangles.data(), count * sizeof(triangle)); !res) return res.error().relay();
      return m;
    } else return f.error().relay();
  }

  bool empty() const noexcept { return triangles.empty(); }
  size_t size() const noexcept { return triangles.size(); }
  auto& operator[](this auto& self, size_t Index) noexcept { return self.triangles[Index]; }
  auto& stl_header(this auto& self) noexcept { return self.header; }

  std::expected<void, error> save(const file::path& Path, bool AllowOverwrite) const {
    const auto mode = AllowOverwrite ? open_mode::create_always : open_mode::create_new;
    if (auto count = static_cast<uint32_t>(triangles.size()); count > std::numeric_limits<uint32_t>::max())
      return std::unexpected(error(errors::invalid_argument, "too many triangles for binary STL"));
    else if (auto f = file_handle::create(Path, mode); !f) return f.error().relay();
    else if (auto res = f->write_exact(header.data(), header.size()); !res) return res.error().relay();
    else if (auto res = f->write_trivial(count); !res) return res.error().relay();
    else if (!triangles.empty()) {
      if (auto res = f->write_exact(triangles.data(), count * sizeof(triangle)); !res) return res.error().relay();
      if (auto res = f->close(); !res) return res.error().relay();
    } else if (auto res = f->close(); !res) return res.error().relay();
    return {};
  }
};
} // namespace yw
