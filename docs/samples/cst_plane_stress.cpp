#include <ywxlib>

using namespace yw;

namespace {

constexpr uint2 canvas_size{980, 620};
constexpr size_t grid_x = 9;
constexpr size_t grid_y = 5;
constexpr size_t node_count = grid_x * grid_y;
constexpr size_t dof_per_node = 2;
constexpr double plate_length = 8.0;
constexpr double plate_height = 2.0;

struct material {
  double young = 210000.0;
  double poisson = 0.30;
  double thickness = 0.10;
};

struct cst_element {
  matrix<double, 3, 6> b{};
  matrix<double, 3, 3> d{};
  double area = 0.0;
};

struct analysis_result {
  vector<double2, node_count> displacement{};
  vector<double3, (grid_x - 1) * (grid_y - 1) * 2> stress{};
  bool solved = false;
  double max_displacement = 0.0;
  double min_mises = 0.0;
  double max_mises = 0.0;
  double2 fixed_reaction{};
  double free_residual_norm = 0.0;
  double total_residual_norm = 0.0;
};

using stiffness_matrix = sparse_matrix<double>;
using element_matrix = matrix<double, 6, 6>;

size_t node_index(size_t x, size_t y) noexcept { return y * grid_x + x; }

double2 node_position(size_t index) noexcept {
  const auto x = index % grid_x;
  const auto y = index / grid_x;
  return {
    plate_length * double(x) / double(grid_x - 1),
    plate_height * double(y) / double(grid_y - 1),
  };
}

vector<uint3, (grid_x - 1) * (grid_y - 1) * 2> make_triangles() {
  vector<uint3, (grid_x - 1) * (grid_y - 1) * 2> triangles{};
  size_t k = 0;
  for (size_t y = 0; y + 1 < grid_y; ++y)
    for (size_t x = 0; x + 1 < grid_x; ++x) {
      const auto n00 = uint32_t(node_index(x, y));
      const auto n10 = uint32_t(node_index(x + 1, y));
      const auto n01 = uint32_t(node_index(x, y + 1));
      const auto n11 = uint32_t(node_index(x + 1, y + 1));
      triangles[k++] = uint3{n00, n10, n11};
      triangles[k++] = uint3{n00, n11, n01};
    }
  return triangles;
}

cst_element make_cst_element(const uint3& tri, const material& mat) {
  const auto p0 = node_position(tri.x);
  const auto p1 = node_position(tri.y);
  const auto p2 = node_position(tri.z);

  const auto twice_area = (p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y);
  const auto area = yw::abs(twice_area) * 0.5;
  const auto inv_2a = 1.0 / twice_area;

  const double beta[3] = {p1.y - p2.y, p2.y - p0.y, p0.y - p1.y};
  const double gamma[3] = {p2.x - p1.x, p0.x - p2.x, p1.x - p0.x};

  cst_element e;
  e.area = area;

  for (size_t i = 0; i < 3; ++i) {
    e.b[0][i * 2] = beta[i] * inv_2a;
    e.b[1][i * 2 + 1] = gamma[i] * inv_2a;
    e.b[2][i * 2] = gamma[i] * inv_2a;
    e.b[2][i * 2 + 1] = beta[i] * inv_2a;
  }

  const auto c = mat.young / (1.0 - mat.poisson * mat.poisson);
  e.d[0][0] = c;
  e.d[0][1] = c * mat.poisson;
  e.d[1][0] = c * mat.poisson;
  e.d[1][1] = c;
  e.d[2][2] = c * (1.0 - mat.poisson) * 0.5;
  return e;
}

element_matrix make_element_stiffness(const uint3& tri, const material& mat) {
  const auto e = make_cst_element(tri, mat);
  auto ke = dot(transpose(e.b), dot(e.d, e.b));
  ke *= mat.thickness * e.area;
  return ke;
}

double von_mises_plane_stress(double3 s) noexcept {
  return yw::sqrt(yw::max(0.0, s.x * s.x - s.x * s.y + s.y * s.y + 3.0 * s.z * s.z));
}

analysis_result analyze() {
  const material mat;
  const auto triangles = make_triangles();
  stiffness_matrix stiffness(node_count, uint1{dof_per_node}, triangles);
  std::vector<double> force(stiffness.size());

  for (const auto& tri : triangles) {
    auto ke = make_element_stiffness(tri, mat);
    if (auto res = add_element_matrix<3, dof_per_node>(stiffness, tri, ke); !res) res.error().go_off();
  }

  constexpr double total_load = -120.0;
  for (size_t y = 0; y < grid_y; ++y) {
    const auto weight = y == 0 || y + 1 == grid_y ? 0.5 : 1.0;
    const auto sum_weight = double(grid_y - 1);
    const auto dof = stiffness.dof_index(node_index(grid_x - 1, y), 1);
    force[dof] += total_load * weight / sum_weight;
  }

  const auto original_stiffness = stiffness;
  const auto original_force = force;

  std::vector<node_dof_constraint<double>> fixed_constraints;
  fixed_constraints.reserve(grid_y * dof_per_node);
  for (size_t y = 0; y < grid_y; ++y) {
    fixed_constraints.push_back({node_index(0, y), 0, 0.0});
    fixed_constraints.push_back({node_index(0, y), 1, 0.0});
  }
  if (auto res = constrain(stiffness, force, fixed_constraints); !res) res.error().go_off();

  analysis_result result;
  auto solved = solve_cg(stiffness, force, 1e-11, 10000);
  if (!solved) return result;

  result.solved = true;
  for (size_t i = 0; i < node_count; ++i) {
    result.displacement[i] = double2{
      (*solved)[stiffness.dof_index(i, 0)],
      (*solved)[stiffness.dof_index(i, 1)],
    };
    result.max_displacement = yw::max(result.max_displacement, result.displacement[i].length());
  }

  auto residual = transform(original_stiffness, *solved);
  for (size_t i = 0; i < residual.size(); ++i) residual[i] -= original_force[i];

  double free_residual2 = 0.0;
  double total_residual2 = 0.0;
  for (size_t i = 0; i < residual.size(); ++i) total_residual2 += residual[i] * residual[i];

  for (size_t y = 0; y < grid_y; ++y) {
    const auto rx = stiffness.dof_index(node_index(0, y), 0);
    const auto ry = stiffness.dof_index(node_index(0, y), 1);
    result.fixed_reaction.x += residual[rx];
    result.fixed_reaction.y += residual[ry];
    residual[rx] = 0.0;
    residual[ry] = 0.0;
  }

  for (const auto r : residual) free_residual2 += r * r;
  result.free_residual_norm = yw::sqrt(free_residual2);
  result.total_residual_norm = yw::sqrt(total_residual2);

  result.min_mises = inf;
  for (size_t ti = 0; ti < triangles.size(); ++ti) {
    const auto& tri = triangles[ti];
    const auto e = make_cst_element(tri, mat);
    vector<double, 6> u{};
    for (size_t i = 0; i < 3; ++i) {
      const auto d = result.displacement[tri[i]];
      u[i * 2] = d.x;
      u[i * 2 + 1] = d.y;
    }
    result.stress[ti] = transform(e.d, transform(e.b, u));
    const auto vm = von_mises_plane_stress(result.stress[ti]);
    result.min_mises = yw::min(result.min_mises, vm);
    result.max_mises = yw::max(result.max_mises, vm);
  }
  return result;
}

color stress_color(double value, double low, double high) {
  const auto t = float(yw::clamp((value - low) / yw::max(high - low, 1e-12), 0.0, 1.0));
  const color a(0.14f, 0.35f, 0.78f);
  const color b(0.10f, 0.65f, 0.56f);
  const color c(0.96f, 0.78f, 0.18f);
  const color d(0.86f, 0.18f, 0.13f);
  if (t < 0.33f) {
    const auto u = t / 0.33f;
    return {a.r + (b.r - a.r) * u, a.g + (b.g - a.g) * u, a.b + (b.b - a.b) * u};
  }
  if (t < 0.66f) {
    const auto u = (t - 0.33f) / 0.33f;
    return {b.r + (c.r - b.r) * u, b.g + (c.g - b.g) * u, b.b + (c.b - b.b) * u};
  }
  const auto u = (t - 0.66f) / 0.34f;
  return {c.r + (d.r - c.r) * u, c.g + (d.g - c.g) * u, c.b + (d.b - c.b) * u};
}

void draw_line(float2 a, float2 b, const color& c, float width = 1.0f) {
  brush::color(c);
  if (auto res = stroke_line(a, b, width); !res) res.error().go_off();
}

void draw_circle(float2 center, float radius, const color& c) {
  brush::color(c);
  if (auto res = fill_ellipse(center, float2::fill(radius)); !res) res.error().go_off();
}

void draw_text_line(float2 pos, string<wchar_t> s, float size = 17.0f, color c = color(0.88f, 0.91f, 0.94f)) {
  text t(std::move(s), {.size = size});
  if (auto res = draw_text(pos, t, c); !res) res.error().go_off();
}

float2 screen_position(size_t index, const analysis_result& result, double scale) {
  const auto p = node_position(index);
  const auto d = result.displacement[index] * scale;
  const auto sx = 100.0f + float((p.x + d.x) / plate_length) * 760.0f;
  const auto sy = 405.0f - float((p.y + d.y) / plate_height) * 230.0f;
  return {sx, sy};
}

float2 original_screen_position(size_t index) {
  const auto p = node_position(index);
  const auto sx = 100.0f + float(p.x / plate_length) * 760.0f;
  const auto sy = 405.0f - float(p.y / plate_height) * 230.0f;
  return {sx, sy};
}

void render(bitmap& target, const analysis_result& result) {
  const auto triangles = make_triangles();
  const auto deformation_scale = result.max_displacement > 0.0 ? 0.55 / result.max_displacement : 1.0;

  if (auto d = target.begin_draw(color(0.052f, 0.059f, 0.066f))) {
    brush::color(color(0.070f, 0.078f, 0.087f));
    if (auto res = fill_rectangle({0.0f, 0.0f}, {float(canvas_size.x), 118.0f}); !res) res.error().go_off();
    if (auto res = fill_rectangle({0.0f, 490.0f}, {float(canvas_size.x), 130.0f}); !res) res.error().go_off();

    draw_text_line({32.0f, 24.0f}, L"CST Plane Stress - sparse CG", 27.0f);
    draw_text_line({32.0f, 66.0f}, format<wchar_t>(L"mesh = ", grid_x, L" x ", grid_y, L" nodes / ", triangles.size(), L" CST elements"), 16.0f);
    draw_text_line({554.0f, 30.0f}, format<wchar_t>(L"max |u| = ", result.max_displacement), 16.0f);
    draw_text_line({554.0f, 56.0f}, format<wchar_t>(L"stress range = ", result.min_mises, L" ... ", result.max_mises), 16.0f);
    draw_text_line({554.0f, 82.0f}, format<wchar_t>(L"display scale = ", deformation_scale), 16.0f);

    for (const auto& tri : triangles) {
      const auto p0 = original_screen_position(tri.x);
      const auto p1 = original_screen_position(tri.y);
      const auto p2 = original_screen_position(tri.z);
      draw_line(p0, p1, color(0.22f, 0.25f, 0.27f), 1.0f);
      draw_line(p1, p2, color(0.22f, 0.25f, 0.27f), 1.0f);
      draw_line(p2, p0, color(0.22f, 0.25f, 0.27f), 1.0f);
    }

    for (size_t ti = 0; ti < triangles.size(); ++ti) {
      const auto& tri = triangles[ti];
      const auto vm = von_mises_plane_stress(result.stress[ti]);
      const auto c = stress_color(vm, result.min_mises, result.max_mises);
      const auto p0 = screen_position(tri.x, result, deformation_scale);
      const auto p1 = screen_position(tri.y, result, deformation_scale);
      const auto p2 = screen_position(tri.z, result, deformation_scale);
      draw_line(p0, p1, c, 3.0f);
      draw_line(p1, p2, c, 3.0f);
      draw_line(p2, p0, c, 3.0f);
    }

    for (size_t y = 0; y < grid_y; ++y) {
      const auto p = screen_position(node_index(0, y), result, deformation_scale);
      draw_circle(p, 4.5f, color(0.75f, 0.78f, 0.82f));
      draw_line(p + float2{-18.0f, -10.0f}, p + float2{-4.0f, 0.0f}, color(0.75f, 0.78f, 0.82f), 1.4f);
      draw_line(p + float2{-18.0f, 10.0f}, p + float2{-4.0f, 0.0f}, color(0.75f, 0.78f, 0.82f), 1.4f);
    }

    for (size_t y = 0; y < grid_y; ++y) {
      const auto p = screen_position(node_index(grid_x - 1, y), result, deformation_scale);
      draw_line(p + float2{0.0f, -28.0f}, p + float2{0.0f, -8.0f}, color(0.92f, 0.39f, 0.20f), 2.2f);
      draw_line(p + float2{0.0f, -8.0f}, p + float2{-7.0f, -17.0f}, color(0.92f, 0.39f, 0.20f), 2.2f);
      draw_line(p + float2{0.0f, -8.0f}, p + float2{7.0f, -17.0f}, color(0.92f, 0.39f, 0.20f), 2.2f);
    }

    const auto legend_x = 108.0f;
    const auto legend_y = 530.0f;
    for (int i = 0; i < 180; ++i) {
      const auto t = double(i) / 179.0;
      brush::color(stress_color(result.min_mises + t * (result.max_mises - result.min_mises), result.min_mises, result.max_mises));
      if (auto res = fill_rectangle({legend_x + float(i) * 2.0f, legend_y}, {2.0f, 18.0f}); !res) res.error().go_off();
    }
    draw_text_line({legend_x, legend_y + 30.0f}, format<wchar_t>(L"von Mises  min ", result.min_mises), 14.0f);
    draw_text_line({legend_x + 256.0f, legend_y + 30.0f}, format<wchar_t>(L"max ", result.max_mises), 14.0f);
    draw_text_line({554.0f, 524.0f}, L"outline: original mesh / colored: deformed element edges", 15.0f, color(0.70f, 0.75f, 0.78f));

    if (auto res = d->close(); !res) res.error().go_off();
  } else d.error().go_off();
}

} // namespace

int main() {
  auto result = analyze();
  if (!result.solved) error(errors::operation_failed, "CST analysis failed").go_off();

  print("fixed reaction = (", result.fixed_reaction.x, ", ", result.fixed_reaction.y, ")");
  print("free residual norm = ", result.free_residual_norm);
  print("total residual norm = ", result.total_residual_norm);

  auto win = window({
    .title = L"CST plane stress",
    .size = int2{int(canvas_size.x), int(canvas_size.y)},
    .resizable = false,
  });
  win.background_color(color(0.052f, 0.059f, 0.066f));

  auto image = bitmap(canvas_size);
  render(image, result);

  while (mainloop) {
    if (auto d = win.begin_draw()) {
      if (auto res = draw_bitmap({}, image); !res) res.error().go_off();
    } else d.error().go_off();
  }
}
