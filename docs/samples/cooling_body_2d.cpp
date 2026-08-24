#include <ywxlib>

#include <vector>

using namespace yw;

namespace {

constexpr double default_fixed_spf = 1.0 / 240.0;

struct simulation_config {
  size_t mesh_width = 64;
  size_t mesh_height = 64;
  double alpha = 700.0;
  double length = 480.0;
  double air_temp = 20.0;
  double initial_temp = 120.0;
  double fixed_spf = default_fixed_spf;

  double dx() const noexcept { return length / double(yw::max(mesh_width, mesh_height) - 1); }
  double stable_dt() const noexcept { return explicit_euler_stable_dt<2>(dx(), alpha); }
  double step_dt() const noexcept { return yw::min(fixed_spf, stable_dt()); }
};

class heat_state {
  size_t _width = 0;
  size_t _height = 0;
  std::vector<double> _temperature;
  std::vector<double> _buffer;

public:
  heat_state() = default;

  heat_state(size_t width, size_t height, double value)
    : _width(width), _height(height), _temperature(width * height, value), _buffer(width * height) {}

  size_t width() const noexcept { return _width; }
  size_t height() const noexcept { return _height; }
  size_t size() const noexcept { return _temperature.size(); }

  double* data() noexcept { return _temperature.data(); }
  const double* data() const noexcept { return _temperature.data(); }
  double* begin() noexcept { return _temperature.data(); }
  const double* begin() const noexcept { return _temperature.data(); }
  double* end() noexcept { return _temperature.data() + _temperature.size(); }
  const double* end() const noexcept { return _temperature.data() + _temperature.size(); }

  std::span<double> buffer(size_t) noexcept { return _buffer; }

  double& operator()(size_t x, size_t y) noexcept { return _temperature[y * _width + x]; }
  double operator()(size_t x, size_t y) const noexcept { return _temperature[y * _width + x]; }
};

struct simulation_state {
  heat_state heat;
  double time = 0.0;
  uint64_t steps = 0;
};

double positive_or(double value, double fallback) noexcept { return value > 0.0 ? value : fallback; }

size_t at_least(size_t value, size_t minimum) noexcept { return yw::max(value, minimum); }

simulation_config parse_config(int argc, char** argv) {
  argument::description = "Cooling of a uniformly hot body in air";
  argument::epilog = "Example: ywlib --w 96 --h 64 --alpha 700 --air 20 --initial 120 --spf 0.004";

  auto width = argument::option<size_t>("--w").description("mesh width").default_value(64);
  auto height = argument::option<size_t>("--h").description("mesh height").default_value(64);
  auto alpha = argument::option<double>("--alpha").description("thermal diffusivity in px^2/s").default_value(700.0);
  auto length = argument::option<double>("--length").description("longer side length in pixels").default_value(480.0);
  auto air = argument::option<double>("--air").description("ambient air temperature").default_value(20.0);
  auto initial =
    argument::option<double>("--initial").description("initial uniform body temperature").default_value(120.0);
  auto spf = argument::option<double>("--spf")
               .description("requested fixed simulation seconds per frame")
               .default_value(default_fixed_spf);

  if (auto res = argument::parse(argc, argv); !res) res.error().go_off();

  return {
    .mesh_width = at_least(width.value(), 3),
    .mesh_height = at_least(height.value(), 3),
    .alpha = positive_or(alpha.value(), 700.0),
    .length = positive_or(length.value(), 480.0),
    .air_temp = air.value(),
    .initial_temp = initial.value(),
    .fixed_spf = positive_or(spf.value(), default_fixed_spf),
  };
}

simulation_state make_initial_state(const simulation_config& config) {
  return {
    .heat = heat_state(config.mesh_width, config.mesh_height, config.initial_temp),
  };
}

void step_simulation(simulation_state& state, const simulation_config& config) {
  const auto dt = config.step_dt();
  const auto coeff = config.alpha / (config.dx() * config.dx());
  const auto width = state.heat.width();
  const auto height = state.heat.height();
  const auto air = config.air_temp;

  auto derivative = [=](const auto& input, std::span<double> out, double) {
    const auto* in = std::ranges::data(input);
    for (size_t y = 0; y < height; ++y) {
      for (size_t x = 0; x < width; ++x) {
        const auto i = y * width + x;
        const auto center = in[i];
        const auto left = x > 0 ? in[i - 1] : air;
        const auto right = x + 1 < width ? in[i + 1] : air;
        const auto top = y > 0 ? in[i - width] : air;
        const auto bottom = y + 1 < height ? in[i + width] : air;
        out[i] = coeff * (left + right + top + bottom - 4.0 * center);
      }
    }
  };

  if (auto res = integrate_euler_inplace(state.heat, state.time, dt, derivative); !res) res.error().go_off();
  state.time += dt;
  ++state.steps;
}

void draw_line(float2 a, float2 b, const color& c, float width = 1.0f) {
  brush::color(c);
  if (auto res = stroke_line(a, b, width); !res) res.error().go_off();
}

double min_temperature(const heat_state& state) {
  double result = *state.begin();
  for (const auto value : state) result = yw::min(result, value);
  return result;
}

double max_temperature(const heat_state& state) {
  double result = *state.begin();
  for (const auto value : state) result = yw::max(result, value);
  return result;
}

color temperature_color(double temp, double low, double high) {
  const auto t = float(yw::clamp((temp - low) / yw::max(high - low, 1.0), 0.0, 1.0));
  const auto cold = color(0.12f, 0.36f, 0.84f);
  const auto mid = color(0.94f, 0.78f, 0.22f);
  const auto hot = color(0.92f, 0.17f, 0.13f);
  if (t < 0.5f) {
    const auto u = t * 2.0f;
    return color(cold.r + (mid.r - cold.r) * u, cold.g + (mid.g - cold.g) * u, cold.b + (mid.b - cold.b) * u);
  }
  const auto u = (t - 0.5f) * 2.0f;
  return color(mid.r + (hot.r - mid.r) * u, mid.g + (hot.g - mid.g) * u, mid.b + (hot.b - mid.b) * u);
}

void update_heat_pixels(std::vector<bgra>& pixels, const simulation_state& state, const simulation_config& config) {
  const auto low = config.air_temp;
  const auto high = yw::max(config.initial_temp, low + 1.0);
  pixels.resize(state.heat.size());
  for (size_t y = 0; y < state.heat.height(); ++y)
    for (size_t x = 0; x < state.heat.width(); ++x)
      pixels[y * state.heat.width() + x] = static_cast<bgra>(temperature_color(state.heat(x, y), low, high));
}

void draw_heat_view(ui::blank& view, const bitmap_texture& heat_bitmap) {
  const auto pos = view.pos();
  const auto size = view.size();
  if (auto res = draw_bitmap(pos, size, heat_bitmap); !res) res.error().go_off();
  draw_line(pos, pos + float2{size.x, 0.0f}, color(0.55f, 0.60f, 0.63f), 1.0f);
  draw_line(pos, pos + float2{0.0f, size.y}, color(0.55f, 0.60f, 0.63f), 1.0f);
  draw_line(pos + float2{size.x, 0.0f}, pos + size, color(0.55f, 0.60f, 0.63f), 1.0f);
  draw_line(pos + float2{0.0f, size.y}, pos + size, color(0.55f, 0.60f, 0.63f), 1.0f);
}

void style_label(ui::label& label, ui::alignment align = ui::left) {
  label.align(align)
    .background_color(colors::transparent)
    .text_color(color(0.88f, 0.92f, 0.96f));
}

struct Gui {
  window win;
  ui::grid_layout root;
  ui::label title;
  ui::label mesh;
  ui::label alpha;
  ui::blank heat_view;
  ui::vlayout stats;
  ui::label air;
  ui::label initial;
  ui::label dt;
  ui::label limit;
  ui::label time;
  ui::label steps;
  ui::label min;
  ui::label max;
  ui::label center;
};

void update_labels(
  Gui& gui,
  const simulation_state& state,
  const simulation_config& config) {
  gui.mesh.string(format<wchar_t>(L"mesh = ", config.mesh_width, L" x ", config.mesh_height));
  gui.alpha.string(format<wchar_t>(L"alpha = ", config.alpha, L"  dx = ", config.dx()));
  gui.air.string(format<wchar_t>(L"air = ", config.air_temp));
  gui.initial.string(format<wchar_t>(L"initial = ", config.initial_temp));
  gui.dt.string(format<wchar_t>(L"dt = ", config.step_dt(), L" s"));
  gui.limit.string(format<wchar_t>(L"limit = ", config.stable_dt(), L" s"));
  gui.time.string(format<wchar_t>(L"time = ", state.time, L" s"));
  gui.steps.string(format<wchar_t>(L"steps = ", state.steps));
  gui.min.string(format<wchar_t>(L"min = ", min_temperature(state.heat)));
  gui.max.string(format<wchar_t>(L"max = ", max_temperature(state.heat)));
  gui.center.string(format<wchar_t>(L"center = ", state.heat(state.heat.width() / 2, state.heat.height() / 2)));
}

} // namespace

int main(int argc, char** argv) {
  const auto config = parse_config(argc, argv);

  Gui gui;

  gui.win = window({
    .title = L"cooling body in air",
    .resizable = false,
  });
  gui.win.background_color(color(0.055f, 0.065f, 0.075f));

  gui.root = ui::grid_layout(gui.win);
  if (auto res = gui.root.columns(ui::fit, ui::fit); !res) res.error().go_off();
  if (auto res = gui.root.rows(ui::fit, ui::fit, ui::fit, ui::fit); !res) res.error().go_off();

  gui.title = ui::label(gui.root.at({0, 0}, {2, 1}));
  gui.mesh = ui::label(gui.root.at({0, 1}));
  gui.alpha = ui::label(gui.root.at({0, 2}));
  gui.heat_view = ui::blank(gui.root.at({0, 3}));
  gui.stats = ui::vlayout(gui.root.at({1, 3}));
  gui.stats.width(240.0f);
  gui.air = ui::label(gui.stats);
  gui.initial = ui::label(gui.stats);
  gui.dt = ui::label(gui.stats);
  gui.limit = ui::label(gui.stats);
  gui.time = ui::label(gui.stats);
  gui.steps = ui::label(gui.stats);
  gui.min = ui::label(gui.stats);
  gui.max = ui::label(gui.stats);
  gui.center = ui::label(gui.stats);

  gui.title.string(L"Cooling Body in Air - D3D Texture");
  style_label(gui.title, ui::center);
  style_label(gui.mesh, ui::center);
  style_label(gui.alpha, ui::center);
  style_label(gui.air);
  style_label(gui.initial);
  style_label(gui.dt);
  style_label(gui.limit);
  style_label(gui.time);
  style_label(gui.steps);
  style_label(gui.min);
  style_label(gui.max);
  style_label(gui.center);
  gui.heat_view.size(float2::fill(480.0f));

  auto heat_gpu = texture(
    int2{int(config.mesh_width), int(config.mesh_height)}, DXGI_FORMAT_B8G8R8A8_UNORM, texture::flag::shader_resource);
  auto heat_bitmap = bitmap_texture(int2{int(config.mesh_width), int(config.mesh_height)});
  auto state = make_initial_state(config);
  std::vector<bgra> heat_pixels;

  update_heat_pixels(heat_pixels, state, config);
  if (auto res = heat_gpu.copy_from(heat_pixels); !res) res.error().go_off();
  if (auto res = heat_bitmap.copy_from(heat_gpu); !res) res.error().go_off();
  update_labels(gui, state, config);

  double accumulator = 0.0;
  while (mainloop) {
    accumulator += yw::min(mainloop.spf(), 0.25);

    bool updated = false;
    const auto max_steps_per_frame = 96;
    for (int i = 0; accumulator >= config.step_dt() && i < max_steps_per_frame; ++i) {
      step_simulation(state, config);
      accumulator -= config.step_dt();
      updated = true;
    }
    if (updated) {
      update_heat_pixels(heat_pixels, state, config);
      if (auto res = heat_gpu.copy_from(heat_pixels); !res) res.error().go_off();
      if (auto res = heat_bitmap.copy_from(heat_gpu); !res) res.error().go_off();
      update_labels(gui, state, config);
    }

    if (auto d = gui.win.begin_draw()) {
      draw_heat_view(gui.heat_view, heat_bitmap);
    } else d.error().go_off();
  }
}
