#include "ywlib"
using namespace yw;

/// double precision vector type
struct alignas(32) wector {
  static constexpr size_t count = 4;
  double x{}, y{}, z{}, w{};
  constexpr wector() noexcept = default;
  explicit constexpr wector(const numerical auto X) noexcept : x(double(X)) {}
  constexpr wector(const numerical auto X, const numerical auto Y) noexcept : x(double(X)), y(double(Y)) {}
  constexpr wector(const numerical auto X, const numerical auto Y,
                   const numerical auto Z) noexcept : x(double(X)), y(double(Y)), z(double(Z)) {}
  constexpr wector(const numerical auto X, const numerical auto Y, const numerical auto Z,
                   const numerical auto W) noexcept : x(double(X)), y(double(Y)), z(double(Z)), w(double(W)) {}
  wector(const __m256d& m) noexcept { _mm256_store_pd(&x, m); }
  wector& operator=(const __m256d& m) noexcept { return _mm256_store_pd(&x, m), *this; }
  operator __m256d() const noexcept { return _mm256_load_pd(&x); }
  constexpr size_t size() const noexcept { return count; }
  constexpr double* data() noexcept { return &x; }
  constexpr const double* data() const noexcept { return &x; }
  constexpr double* begin() noexcept { return &x; }
  constexpr const double* begin() const noexcept { return &x; }
  constexpr double* end() noexcept { return &w + 1; }
  constexpr const double* end() const noexcept { return &w + 1; }
  constexpr double& operator[](const size_t i) noexcept {
    if (is_cev) {
      if (i == 0) return x;
      else if (i == 1) return y;
      else if (i == 2) return z;
      else return w;
    } else return (&x)[i];
  }
  constexpr const double& operator[](const size_t i) const noexcept {
    if (is_cev) {
      if (i == 0) return x;
      else if (i == 1) return y;
      else if (i == 2) return z;
      else return w;
    } else return (&x)[i];
  }
  template<size_t I> constexpr double& get() noexcept { return parameter_switch<I>(x, y, z, w); }
  template<size_t I> constexpr const double& get() const noexcept { return parameter_switch<I>(x, y, z, w); }
  friend constexpr bool operator==(const wector& a, const wector& b) noexcept {
    if (is_cev) return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
    else return xveq(a, b);
  }
  friend constexpr auto operator<=>(const wector& a, const wector& b) noexcept {
    if (is_cev) {
      if (auto c = a.x <=> b.x; c != 0) return c;
      if (auto c = a.y <=> b.y; c != 0) return c;
      if (auto c = a.z <=> b.z; c != 0) return c;
      return a.w <=> b.w;
    } else return std::partial_ordering(xvtw(a, b));
  }
  friend constexpr wector operator+(const wector& a) noexcept { return a; }
  friend constexpr wector operator-(const wector& a) noexcept {
    if (is_cev) return wector(-a.x, -a.y, -a.z, -a.w);
    else return xvneg(a);
  }
  friend constexpr wector operator+(const wector& a, const wector& b) noexcept {
    if (is_cev) return wector(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
    else return xvadd(a, b);
  }
  friend constexpr wector operator-(const wector& a, const wector& b) noexcept {
    if (is_cev) return wector(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
    else return xvsub(a, b);
  }
  friend constexpr wector operator*(const wector& a, const wector& b) noexcept {
    if (is_cev) return wector(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
    else return xvmul(a, b);
  }
  friend constexpr wector operator/(const wector& a, const wector& b) noexcept {
    if (is_cev) return wector(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
    else return xvdiv(a, b);
  }
  friend constexpr wector& operator+=(wector& a, const wector& b) noexcept { return a = a + b; }
  friend constexpr wector& operator-=(wector& a, const wector& b) noexcept { return a = a - b; }
  friend constexpr wector& operator*=(wector& a, const wector& b) noexcept { return a = a * b; }
  friend constexpr wector& operator/=(wector& a, const wector& b) noexcept { return a = a / b; }
  friend constexpr wector operator+(const wector& a, const numerical auto b) noexcept {
    if (is_cev) return wector(a.x + b, a.y + b, a.z + b, a.w + b);
    else return xvadd(a, xvload(double(b)));
  }
  friend constexpr wector operator-(const wector& a, const numerical auto b) noexcept {
    if (is_cev) return wector(a.x - b, a.y - b, a.z - b, a.w - b);
    else return xvsub(a, xvload(double(b)));
  }
  friend constexpr wector operator*(const wector& a, const numerical auto b) noexcept {
    if (is_cev) return wector(a.x * b, a.y * b, a.z * b, a.w * b);
    else return xvmul(a, xvload(double(b)));
  }
  friend constexpr wector operator/(const wector& a, const numerical auto b) noexcept {
    if (is_cev) return wector(a.x / b, a.y / b, a.z / b, a.w / b);
    else return xvdiv(a, xvload(double(b)));
  }
  friend constexpr wector& operator+=(wector& a, const numerical auto b) noexcept { return a = a + b; }
  friend constexpr wector& operator-=(wector& a, const numerical auto b) noexcept { return a = a - b; }
  friend constexpr wector& operator*=(wector& a, const numerical auto b) noexcept { return a = a * b; }
  friend constexpr wector& operator/=(wector& a, const numerical auto b) noexcept { return a = a / b; }
  friend constexpr wector operator+(const numerical auto a, const wector& b) noexcept {
    if (is_cev) return wector(a + b.x, a + b.y, a + b.z, a + b.w);
    else return xvadd(xvload(double(a)), b);
  }
  friend constexpr wector operator-(const numerical auto a, const wector& b) noexcept {
    if (is_cev) return wector(a - b.x, a - b.y, a - b.z, a - b.w);
    else return xvsub(xvload(double(a)), b);
  }
  friend constexpr wector operator*(const numerical auto a, const wector& b) noexcept {
    if (is_cev) return wector(a * b.x, a * b.y, a * b.z, a * b.w);
    else return xvmul(xvload(double(a)), b);
  }
  friend constexpr wector operator/(const numerical auto a, const wector& b) noexcept {
    if (is_cev) return wector(a / b.x, a / b.y, a / b.z, a / b.w);
    else return xvdiv(xvload(double(a)), b);
  }
};

struct single_precision_facet {
  vector origin;
  vector vertex[3];
  vector edge[3];
};

constexpr float a = 2001.085511;

struct double_precision_facet {
  wector vertex[3];
  single_precision_facet to_single_precision() const noexcept {
    single_precision_facet result;
    auto o = xvmul(xvadd(xvadd(vertex[0], vertex[1]), vertex[2]), xv_constant<1 / 3.0>);
    _mm_store_ps(&result.origin.x, xvcast<__m128>(o));
    _mm_store_ps(&result.vertex[0].x, xvcast<__m128>(xvsub(vertex[0], o)));
    _mm_store_ps(&result.vertex[1].x, xvcast<__m128>(xvsub(vertex[1], o)));
    _mm_store_ps(&result.vertex[2].x, xvcast<__m128>(xvsub(vertex[2], o)));
    _mm_store_ps(&result.edge[0].x, xvcast<__m128>(xvsub(result.vertex[1], result.vertex[0])));
    _mm_store_ps(&result.edge[1].x, xvcast<__m128>(xvsub(result.vertex[2], result.vertex[1])));
    _mm_store_ps(&result.edge[2].x, xvcast<__m128>(xvsub(result.vertex[0], result.vertex[2])));
    return result;
  }
};


struct facet {
  vector origin;
  vector vertex[3];
  vector edge[3];
};

int main() {
  main::resize(800, 600);
  auto label = yw::label({0, 0, 100, 20}, L"Hello, World!");
  label.show();
  key::escape.down = [] { main::terminate(); };

  D3D11_FEATURE_DATA_DOUBLES featureData;
  HRESULT hr = main::d3d_device->CheckFeatureSupport(D3D11_FEATURE_DOUBLES, &featureData, sizeof(featureData));

  if (SUCCEEDED(hr)) {
    if (featureData.DoublePrecisionFloatShaderOps) {
      std::cout << "OK" << std::endl;
    } else {
      std::cout << "NO" << std::endl;
    }
  }
}
