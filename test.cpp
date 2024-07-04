#include "ywlib"
using namespace yw;


class stl {
protected:
  array<cat1> _{};
public:
#pragma pack(2)
  /// representing a normal vector, three vertices, and an attribute of each triangle
  struct facet {
    std::array<float, 3> normal;
    std::array<std::array<float, 3>, 3> vertex;
    unsigned short attribute;
  };
#pragma pack()

  /// default constructor
  stl() = default;

  /// constructor from .stl file
  stl(const std::filesystem::path& p) : _(std::filesystem::file_size(p)) { file::read(p, _); }

  /// constructs a stl data with n facets
  stl(nat n) : _(84 + 50 * n) { *reinterpret_cast<unsigned*>(std::memset(_.data(), ' ', 80)) = unsigned(n); }

  /// returns the number of facets
  unsigned size() const { return *reinterpret_cast<const unsigned*>(_.data() + 80); }

  /// checks if the data is empty
  bool empty() const { return size() == 0 || _.size() != 84 + 50 * size(); }

  /// returns the string view of the file header
  std::string_view header() const { return {_.data(), 80}; }

  /// sets the file header
  void header(const std::string_view& s) { std::char_traits<char>::copy(_.data(), s.data(), min(s.size(), 80)); }

  /// returns the pointer to the first facet
  facet* data() { return reinterpret_cast<facet*>(_.data() + 84); }
  const facet* data() const { return reinterpret_cast<const facet*>(_.data() + 84); }

  /// returns the iterator to the first facet
  facet* begin() { return data(); }
  const facet* begin() const { return data(); }

  /// returns the iterator to the end of the facets
  facet* end() { return data() + size(); }
  const facet* end() const { return data() + size(); }

  /// returns the reference to the i-th facet
  facet& operator[](nat i) { return data()[i]; }
  const facet& operator[](nat i) const { return data()[i]; }

  /// writes the data to a file
  void write(const path& p) const { file::write(p, _); }
};

/// creates facets from stl data
unordered_buffer<array<fat4, 28>> make_facets(const stl& Stl) {
  try {
    static constexpr string_view<cat1> hlsl = R"(
#pragma pack_matrix(row_major)
struct Facet { float3 a, b, c; float3 ab, bc, ca; float3 sq, nisq; float4 plane; };
RWStructuredBuffer<Facet> Facets : register(u0);
[numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
  Facets[i].ab = Facets[i].b - Facets[i].a;
  Facets[i].bc = Facets[i].c - Facets[i].b;
  Facets[i].ca = Facets[i].a - Facets[i].c;
  Facets[i].sq.x = dot(Facets[i].ab, Facets[i].ab);
  Facets[i].sq.y = dot(Facets[i].bc, Facets[i].bc);
  Facets[i].sq.z = dot(Facets[i].ca, Facets[i].ca);
  Facets[i].nisq = -1.0 / Facets[i].sq;
  float4 t = float4(cross(Facets[i].a, Facets[i].b), 0);
  t = float4(t.xyz + cross(Facets[i].b, Facets[i].c) + cross(Facets[i].c, Facets[i].a), dot(t.xyz, Facets[i].c));
  Facets[i].plane = t / length(t.xyz);
})";
    if (Stl.empty()) throw std::runtime_error("The stl data is empty.");
    array<array<fat4, 28>> temp(Stl.size());
    for (nat i = 0; i < temp.size(); ++i) {
      std::memcpy(temp[i].data(), &Stl[i].vertex, 9 * sizeof(fat4));
    }
    unordered_buffer<array<fat4, 28>> f(temp);
    computer<typepack<decltype(f)>> c(hlsl);
    c(f.count, {f}, {}, {});
    return f;
  } catch (const std::exception& E) {
    yw::log.error(E.what());
    return {};
  }
}

int main() {
  try {
    auto mm = xmatrix{xvload(1.f, 2.f, 3.f, 4.f), xvload(5.f, 6.f, 7.f, 8.f), xvload(9.f, 10.f, 11.f, 12.f), xvload(13.f, 14.f, 15.f, 16.f)};
    xmatrix nn;
    xvtranspose(mm, nn);
    std::cout << vector(nn[0]) << std::endl;
    std::cout << vector(nn[1]) << std::endl;
    std::cout << vector(nn[2]) << std::endl;
    std::cout << vector(nn[3]) << std::endl;
    std::cout << vector(xvpermute<1, 5, -1, -1>(mm[0], mm[1])) << std::endl;
    std::cout << vector(t_xvpermute<1, 5, 2, 3>::call(mm[0])) << std::endl;

    stl Stl(R"(C:\Users\AA1W017\Documents\Z20335D-3　5月24日_標準.stl)");
    auto ub_facets = make_facets(Stl);
    auto sb_facets = structured_buffer(ub_facets);
    auto test = sb_facets.to_cpu();
    // for (nat i = 0; i < 100; ++i) {
    //   for (nat j = 0; j < 28; ++j) {
    //     std::cout << test[i][j] << ' ';
    //   }
    //   std::cout << std::endl;
    // }

    array<list<vector, vector>> Rays(1);
    Rays[0] = {vector{0, 2200, 0}, vector{0, -1, 0}};
    structured_buffer sb_rays(Rays);
    xmatrix World = xv_identity;
    unordered_buffer<vector> ub_intersections(sb_rays.count);
    unordered_buffer<fat4> ub_distances(sb_rays.count);
    calc_intersection_line_to_facets(ub_intersections, ub_distances, sb_rays, sb_facets, World);
    auto result_intersections = ub_intersections.to_cpu();
    auto result_distances = ub_distances.to_cpu();
    std::cout << result_intersections[0] << ' ' << result_distances[0] << std::endl;
  } catch (const std::exception& E) {
    std::cout << E.what();
    return -1;
  }
  return 0;
}
