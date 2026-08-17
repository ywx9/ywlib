#include <ywxlib>

using namespace yw;

namespace {

constexpr uint2 canvas_size{900, 420};
constexpr double default_fixed_spf = 1.0 / 120.0;
constexpr double two_pi = 6.28318530717958647692;
constexpr size_t free_node_count = 8;
constexpr size_t system_size = free_node_count * 2;

struct simulation_config {
  double initial_offset = 0.0;
  double initial_velocity = 0.0;
  double mass = 1.0;
  double spring = 80.0;
  double damping = 2.0;
  double force_amplitude = 6000.0;
  double force_frequency = 0.65;
  double fixed_spf = default_fixed_spf;
};

struct simulation_state {
  vector<double, free_node_count> displacement{};
  vector<double, free_node_count> velocity{};
  double time = 0.0;
  uint64_t steps = 0;

  double end_offset() const noexcept { return displacement[free_node_count - 1]; }
  double end_velocity() const noexcept { return velocity[free_node_count - 1]; }
};

double positive_or(double value, double fallback) noexcept {
  return value > 0.0 ? value : fallback;
}

simulation_config parse_config(int argc, char** argv) {
  argument::description = "Forced vibration of a spring-mass chain";
  argument::epilog =
    "Example: ywlib --force 8000 --freq 0.8 --m 1.2 --k 100 --c 2.5 --spf 0.008333";

  auto x0 = argument::option<double>("--x0")
              .alias("-x")
              .description("initial offset from natural length in pixels")
              .default_value(0.0);
  auto v0 = argument::option<double>("--v0")
              .alias("-v")
              .description("initial velocity in pixels per second")
              .default_value(0.0);
  auto mass = argument::option<double>("--m")
                .description("mass")
                .default_value(1.0);
  auto spring = argument::option<double>("--k")
                  .description("spring constant")
                  .default_value(80.0);
  auto damping = argument::option<double>("--c")
                   .description("damping coefficient")
                   .default_value(2.0);
  auto force = argument::option<double>("--force")
                 .alias("-f")
                 .description("sinusoidal force amplitude applied to the right end")
                 .default_value(6000.0);
  auto freq = argument::option<double>("--freq")
                .description("sinusoidal force frequency in Hz")
                .default_value(0.65);
  auto spf = argument::option<double>("--spf")
               .description("fixed simulation seconds per frame")
               .default_value(default_fixed_spf);

  argument::parse(argc, argv);

  return {
    .initial_offset = x0.value(),
    .initial_velocity = v0.value(),
    .mass = positive_or(mass.value(), 1.0),
    .spring = positive_or(spring.value(), 80.0),
    .damping = yw::max(0.0, damping.value()),
    .force_amplitude = force.value(),
    .force_frequency = positive_or(freq.value(), 0.65),
    .fixed_spf = positive_or(spf.value(), default_fixed_spf),
  };
}

double end_force(double time, const simulation_config& config) noexcept {
  return config.force_amplitude * yw::sin(two_pi * config.force_frequency * time);
}

void step_simulation(simulation_state& state, const simulation_config& config) {
  const auto dt = config.fixed_spf;
  const auto element_stiffness = config.spring * double(free_node_count);
  const auto force = end_force(state.time + dt, config);

  matrix<double, free_node_count, free_node_count> stiffness{};
  for (size_t element = 0; element < free_node_count; ++element) {
    const auto left_node = element;
    const auto right_node = element + 1;

    if (left_node > 0) stiffness[left_node - 1][left_node - 1] += element_stiffness;
    stiffness[right_node - 1][right_node - 1] += element_stiffness;
    if (left_node > 0) {
      stiffness[left_node - 1][right_node - 1] -= element_stiffness;
      stiffness[right_node - 1][left_node - 1] -= element_stiffness;
    }
  }

  matrix<double, system_size, system_size> a{};
  vector<double, system_size> b{};

  for (size_t i = 0; i < free_node_count; ++i) {
    a[i][i] = 1.0;
    a[i][i + free_node_count] = -dt;
    b[i] = state.displacement[i];

    for (size_t j = 0; j < free_node_count; ++j) a[i + free_node_count][j] = dt * stiffness[i][j];
    a[i + free_node_count][i + free_node_count] = config.mass + dt * config.damping;
    b[i + free_node_count] = config.mass * state.velocity[i];
  }
  b[system_size - 1] += dt * force;

  const auto next = solve(a, b);
  for (size_t i = 0; i < free_node_count; ++i) {
    state.displacement[i] = next[i];
    state.velocity[i] = next[i + free_node_count];
  }
  state.time += dt;
  ++state.steps;
}

void draw_line(float2 a, float2 b, const color& c, float width = 1.0f) {
  brush::color(c);
  if (auto res = stroke_line(a, b, width); !res) res.error().go_off();
}

void draw_rect(float2 pos, float2 size, const color& c) {
  brush::color(c);
  if (auto res = fill_rectangle(pos, size); !res) res.error().go_off();
}

void draw_circle(float2 center, float radius, const color& c) {
  brush::color(c);
  if (auto res = fill_ellipse(center, float2::fill(radius)); !res) res.error().go_off();
}

void draw_spring(float2 start, float2 end, int coils = 3, float amplitude = 8.0f) {
  const float lead = 8.0f;
  const auto dx = end.x - start.x;

  if (dx <= lead * 2.0f) {
    draw_line(start, end, color(0.92f, 0.71f, 0.24f), 2.0f);
    return;
  }

  float2 prev = start;
  draw_line(prev, {start.x + lead, start.y}, color(0.92f, 0.71f, 0.24f), 2.0f);
  prev = {start.x + lead, start.y};

  const auto body = dx - lead * 2.0f;
  const int segments = coils * 2;
  for (int i = 1; i <= segments; ++i) {
    const auto t = float(i) / float(segments);
    const auto x = start.x + lead + body * t;
    const auto y = start.y + ((i % 2 == 0) ? -amplitude : amplitude);
    const float2 next{x, y};
    draw_line(prev, next, color(0.92f, 0.71f, 0.24f), 2.0f);
    prev = next;
  }

  const float2 spring_end{end.x - lead, start.y};
  draw_line(prev, spring_end, color(0.92f, 0.71f, 0.24f), 2.0f);
  draw_line(spring_end, end, color(0.92f, 0.71f, 0.24f), 2.0f);
}

void draw_marker(float x, float top, float bottom, const color& c) {
  brush::dashed(true);
  draw_line({x, top}, {x, bottom}, c, 1.5f);
  brush::dashed(false);
}

void draw_text_line(float2 pos, string<wchar_t> s, float size = 17.0f) {
  text t(std::move(s), {.size = size});
  t.color(color(0.86f, 0.90f, 0.94f));
  if (auto res = draw_text(pos, t); !res) res.error().go_off();
}

void render_simulation(bitmap& target, const simulation_state& state, const simulation_config& config) {
  const auto width = float(canvas_size.x);
  const auto height = float(canvas_size.y);
  const auto wall_x = 82.0f;
  const auto rest_end_x = width - 112.0f;
  const auto center_y = height * 0.56f;
  const auto element_length = (rest_end_x - wall_x) / float(free_node_count);
  const auto force = end_force(state.time, config);

  vector<float2, free_node_count + 1> nodes{};
  nodes[0] = {wall_x, center_y};
  for (size_t i = 1; i <= free_node_count; ++i) {
    const auto rest_x = wall_x + element_length * float(i);
    const auto x = yw::clamp(rest_x + float(state.displacement[i - 1]), wall_x + 24.0f, width - 46.0f);
    nodes[i] = {x, center_y};
  }

  if (auto d = target.begin_draw(color(0.055f, 0.065f, 0.075f))) {
    draw_rect({0.0f, height - 74.0f}, {width, 74.0f}, color(0.075f, 0.085f, 0.095f));
    draw_line({44.0f, center_y + 43.0f}, {width - 44.0f, center_y + 43.0f}, color(0.24f, 0.28f, 0.31f), 2.0f);

    draw_rect({wall_x - 18.0f, center_y - 100.0f}, {18.0f, 200.0f}, color(0.72f, 0.76f, 0.78f));
    for (int i = 0; i < 9; ++i) {
      const auto y = center_y - 92.0f + float(i) * 24.0f;
      draw_line({wall_x - 18.0f, y + 18.0f}, {wall_x - 2.0f, y}, color(0.39f, 0.43f, 0.45f), 1.0f);
    }

    for (size_t i = 1; i <= free_node_count; ++i) {
      const auto rest_x = wall_x + element_length * float(i);
      draw_marker(rest_x, center_y - 58.0f, center_y + 58.0f, color(0.34f, 0.78f, 0.84f, 0.28f));
    }

    for (size_t i = 0; i < free_node_count; ++i) draw_spring(nodes[i], nodes[i + 1]);

    for (size_t i = 1; i <= free_node_count; ++i) {
      const bool end_node = i == free_node_count;
      const auto radius = end_node ? 16.0f : 10.0f;
      draw_circle(nodes[i] + float2{4.0f, 5.0f}, radius, color(0.0f, 0.0f, 0.0f, 0.24f));
      draw_circle(nodes[i], radius, end_node ? color(0.84f, 0.36f, 0.29f) : color(0.31f, 0.58f, 0.80f));
      draw_text_line(nodes[i] + float2{-8.0f, 22.0f}, format<wchar_t>(i), 12.0f);
    }

    const auto force_scale = config.force_amplitude != 0.0 ? yw::clamp(float(force / config.force_amplitude), -1.0f, 1.0f) : 0.0f;
    const auto arrow_start = nodes[free_node_count] + float2{24.0f, -32.0f};
    const auto arrow_end = arrow_start + float2{force_scale * 64.0f, 0.0f};
    draw_line(arrow_start, arrow_end, color(0.93f, 0.55f, 0.22f), 3.0f);
    draw_line(arrow_end, arrow_end + float2{-10.0f * (force_scale >= 0.0f ? 1.0f : -1.0f), -7.0f}, color(0.93f, 0.55f, 0.22f), 3.0f);
    draw_line(arrow_end, arrow_end + float2{-10.0f * (force_scale >= 0.0f ? 1.0f : -1.0f), 7.0f}, color(0.93f, 0.55f, 0.22f), 3.0f);

    draw_text_line({28.0f, 22.0f}, L"Forced Spring-Mass Chain", 25.0f);
    draw_text_line({28.0f, 64.0f}, format<wchar_t>(L"end x = ", int(yw::round(state.end_offset())), L" px"));
    draw_text_line({28.0f, 91.0f}, format<wchar_t>(L"end v = ", int(yw::round(state.end_velocity())), L" px/s"));
    draw_text_line({28.0f, 118.0f}, format<wchar_t>(L"force = ", int(yw::round(force))));
    draw_text_line({28.0f, 145.0f}, format<wchar_t>(L"t = ", int(yw::round(state.time * 10.0)) / 10.0, L" s"));
    draw_text_line(
      {28.0f, 177.0f},
      format<wchar_t>(
        L"u1=", int(yw::round(state.displacement[0])), L"  u2=", int(yw::round(state.displacement[1])),
        L"  u3=", int(yw::round(state.displacement[2])), L"  u4=", int(yw::round(state.displacement[3]))),
      14.0f);
    draw_text_line(
      {28.0f, 200.0f},
      format<wchar_t>(
        L"u5=", int(yw::round(state.displacement[4])), L"  u6=", int(yw::round(state.displacement[5])),
        L"  u7=", int(yw::round(state.displacement[6])), L"  u8=", int(yw::round(state.displacement[7])),
        L" px"),
      14.0f);

    draw_text_line(
      {width - 326.0f, 24.0f},
      format<wchar_t>(free_node_count, L" masses / ", free_node_count, L" elements"),
      15.0f);
    draw_text_line(
      {width - 326.0f, 48.0f},
      format<wchar_t>(L"m=", config.mass, L"  k=", config.spring, L"  c=", config.damping),
      15.0f);
    draw_text_line(
      {width - 326.0f, 72.0f},
      format<wchar_t>(L"F=", config.force_amplitude, L"  freq=", config.force_frequency, L" Hz"),
      15.0f);
    draw_text_line(
      {width - 326.0f, 96.0f},
      format<wchar_t>(L"element k=", config.spring * double(free_node_count), L"  steps=", state.steps),
      15.0f);

    if (auto res = d->close(); !res) res.error().go_off();
  } else d.error().go_off();
}

} // namespace

int main(int argc, char** argv) {
  const auto config = parse_config(argc, argv);

  auto win = window({
    .title = L"forced spring-mass chain",
    .size = int2{int(canvas_size.x), int(canvas_size.y)},
    .resizable = false,
  });
  win.background_color(color(0.055f, 0.065f, 0.075f));

  auto sim_bitmap = bitmap(canvas_size);
  simulation_state state{};
  for (size_t i = 0; i < free_node_count; ++i) {
    const auto t = double(i + 1) / double(free_node_count);
    state.displacement[i] = config.initial_offset * t;
    state.velocity[i] = config.initial_velocity * t;
  }

  double accumulator = 0.0;
  render_simulation(sim_bitmap, state, config);

  while (mainloop) {
    accumulator += yw::min(mainloop.spf(), 0.25);

    bool updated = false;
    const auto max_steps_per_frame = 32;
    for (int i = 0; accumulator >= config.fixed_spf && i < max_steps_per_frame; ++i) {
      step_simulation(state, config);
      accumulator -= config.fixed_spf;
      updated = true;
    }
    if (updated) render_simulation(sim_bitmap, state, config);

    if (auto d = win.begin_draw()) {
      if (auto res = draw_bitmap({}, sim_bitmap); !res) res.error().go_off();
    } else d.error().go_off();
  }
}
