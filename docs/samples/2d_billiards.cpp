#include <ywxlib>

using namespace yw;

namespace {

constexpr float base_table_width = 860.0f;
constexpr float base_table_height = 500.0f;
constexpr float base_rail = 34.0f;
constexpr float base_ball_radius = 13.0f;
constexpr float base_pocket_radius = 28.0f;
constexpr float base_cue_power = 720.0f;
constexpr float felt_drag = 0.48f;
constexpr float base_stop_speed = 6.0f;

struct table_config {
  float scale = 1.0f;
  int total_ball_count = 16;
  float table_width = base_table_width;
  float table_height = base_table_height;
  float rail = base_rail;
  float ball_radius = base_ball_radius;
  float pocket_radius = base_pocket_radius;
  float cue_power = base_cue_power;
  float stop_speed = base_stop_speed;

  int object_ball_count() const noexcept { return yw::max(total_ball_count - 1, 0); }
  uint2 bitmap_size() const noexcept { return {uint32_t(table_width), uint32_t(table_height)}; }
};

table_config make_config(float scale, int total_ball_count) {
  table_config cfg;
  cfg.scale = yw::clamp(scale, 0.35f, 2.0f);
  cfg.total_ball_count = yw::clamp(total_ball_count, 2, 46);
  cfg.table_width = base_table_width * cfg.scale;
  cfg.table_height = base_table_height * cfg.scale;
  cfg.rail = base_rail * cfg.scale;
  cfg.ball_radius = base_ball_radius * cfg.scale;
  cfg.pocket_radius = base_pocket_radius * cfg.scale;
  cfg.cue_power = base_cue_power * cfg.scale;
  cfg.stop_speed = base_stop_speed * cfg.scale;
  return cfg;
}

enum class play_phase {
  playing,
  cue_pocketed,
  clear,
};

struct ball {
  phy::circle_body<float> body;
  color fill;
  bool striped = false;
  bool active = true;
};

struct game_state {
  table_config cfg;
  std::vector<ball> balls;
  float2 pointer{};
  bool aiming = false;
  int shots = 0;
  play_phase phase = play_phase::playing;

  phy::aabb<float> bounds() const noexcept {
    return {
      {cfg.rail + cfg.ball_radius, cfg.rail + cfg.ball_radius},
      {cfg.table_width - cfg.rail - cfg.ball_radius, cfg.table_height - cfg.rail - cfg.ball_radius},
    };
  }

  bool moving() const {
    return std::ranges::any_of(balls, [&](const ball& b) {
      return b.active && b.body.velocity.squared_length() > cfg.stop_speed * cfg.stop_speed;
    });
  }

  void rack() {
    balls.clear();
    const color colors[] = {
      color(0.95f, 0.91f, 0.78f),
      color(0.95f, 0.79f, 0.22f),
      color(0.13f, 0.42f, 0.80f),
      color(0.86f, 0.22f, 0.22f),
      color(0.45f, 0.26f, 0.72f),
      color(0.92f, 0.50f, 0.18f),
      color(0.14f, 0.55f, 0.34f),
      color(0.58f, 0.17f, 0.16f),
      color(0.08f, 0.08f, 0.09f),
    };

    balls.push_back({
      .body = {.position = {cfg.table_width * 0.27f, cfg.table_height * 0.5f}, .radius = cfg.ball_radius, .mass = 1.0f},
      .fill = colors[0],
    });

    const auto start = float2{cfg.table_width * 0.67f, cfg.table_height * 0.5f};
    int number = 1;
    for (int row = 0; number <= cfg.object_ball_count(); ++row) {
      for (int col = 0; col <= row; ++col) {
        if (number > cfg.object_ball_count()) break;
        const auto x = start.x + float(row) * cfg.ball_radius * 1.76f;
        const auto y = start.y + (float(col) - float(row) * 0.5f) * cfg.ball_radius * 2.08f;
        balls.push_back({
          .body = {.position = {x, y}, .radius = cfg.ball_radius, .mass = 1.0f},
          .fill = colors[(number - 1) % 8 + 1],
          .striped = number > 8,
        });
        ++number;
      }
    }
    shots = 0;
    aiming = false;
    phase = play_phase::playing;
  }

  void continue_after_cue_pocketed() {
    auto& cue = balls.front();
    cue.active = true;
    cue.body.position = {cfg.table_width * 0.27f, cfg.table_height * 0.5f};
    cue.body.velocity = {};
    aiming = false;
    phase = play_phase::playing;
  }
};

struct input_state {
  bool shoot = false;
  bool reset = false;
  bool continue_game = false;
  bool new_game = false;
};

float2 to_table(ui::blank& canvas, short2 p) {
  return float2{float(p.x), float(p.y)} - canvas.pos();
}

std::array<float2, 6> pocket_centers(const table_config& cfg) {
  return {
    float2{cfg.rail, cfg.rail},
    float2{cfg.table_width * 0.5f, cfg.rail * 0.82f},
    float2{cfg.table_width - cfg.rail, cfg.rail},
    float2{cfg.rail, cfg.table_height - cfg.rail},
    float2{cfg.table_width * 0.5f, cfg.table_height - cfg.rail * 0.82f},
    float2{cfg.table_width - cfg.rail, cfg.table_height - cfg.rail},
  };
}

int remaining_object_balls(const game_state& game) {
  return int(std::ranges::count_if(game.balls.begin() + 1, game.balls.end(), [](const ball& b) { return b.active; }));
}

void stop_balls(game_state& game) {
  for (auto& b : game.balls) b.body.velocity = {};
}

void sink_pocketed_balls(game_state& game) {
  bool cue_sank = false;
  for (auto& b : game.balls) {
    if (!b.active) continue;
    for (const auto p : pocket_centers(game.cfg)) {
      if ((b.body.position - p).length() > game.cfg.pocket_radius * 0.82f) continue;
      if (&b == &game.balls.front()) {
        b.active = false;
        b.body.velocity = {};
        cue_sank = true;
      } else {
        b.active = false;
        b.body.velocity = {};
      }
      break;
    }
  }

  if (remaining_object_balls(game) == 0) {
    game.phase = play_phase::clear;
    game.aiming = false;
    stop_balls(game);
  } else if (cue_sank) {
    game.phase = play_phase::cue_pocketed;
    game.aiming = false;
    stop_balls(game);
  }
}

void update_game(game_state& game, input_state& input, float dt) {
  if (input.reset || input.new_game) {
    game.rack();
    input.reset = false;
    input.new_game = false;
    input.shoot = false;
    input.continue_game = false;
    return;
  }

  if (input.continue_game) {
    if (game.phase == play_phase::cue_pocketed) game.continue_after_cue_pocketed();
    input.continue_game = false;
    input.shoot = false;
    return;
  }

  if (game.phase != play_phase::playing) {
    input.shoot = false;
    return;
  }

  const auto any_moving = game.moving();
  auto& cue = game.balls.front();
  game.aiming = !any_moving && cue.active;

  if (input.shoot && game.aiming) {
    auto dir = cue.body.position - game.pointer;
    const auto len = dir.length();
    if (len > 14.0f) {
      dir /= len;
      const auto max_pull = 128.0f * game.cfg.scale;
      const auto pull = yw::min(len, max_pull);
      cue.body.velocity = dir * (game.cfg.cue_power * pull / max_pull);
      ++game.shots;
    }
  }
  input.shoot = false;

  const auto bounds = game.bounds();
  for (auto& b : game.balls) {
    if (!b.active) continue;
    b.body.position = phy::integrate(b.body.position, b.body.velocity, dt);
    b.body.velocity = phy::damp(b.body.velocity, felt_drag, dt);
    if (b.body.velocity.length() < game.cfg.stop_speed) b.body.velocity = {};
    phy::resolve_bounds(b.body, bounds, 0.93f);
  }

  for (int pass = 0; pass < 3; ++pass) {
    for (size_t i = 0; i < game.balls.size(); ++i) {
      if (!game.balls[i].active) continue;
      for (size_t j = i + 1; j < game.balls.size(); ++j) {
        if (!game.balls[j].active) continue;
        phy::resolve_collision(game.balls[i].body, game.balls[j].body, 0.96f);
      }
    }
  }

  sink_pocketed_balls(game);
}

void fill_rect(float2 origin, float2 pos, float2 size, const color& c) {
  brush::color(c);
  if (auto res = fill_rectangle(origin + pos, size); !res) res.error().go_off();
}

void stroke_rect(float2 origin, float2 pos, float2 size, const color& c, float width = 1.0f) {
  brush::color(c);
  if (auto res = stroke_rectangle(origin + pos, size, width); !res) res.error().go_off();
}

void line(float2 origin, float2 a, float2 b, const color& c, float width = 1.0f) {
  brush::color(c);
  if (auto res = stroke_line(origin + a, origin + b, width); !res) res.error().go_off();
}

void ellipse(float2 origin, float2 center, float radius, const color& c) {
  brush::color(c);
  if (auto res = fill_ellipse(origin + center, float2::fill(radius)); !res) res.error().go_off();
}

void draw_ball(float2 origin, const ball& b, int number) {
  if (!b.active) return;
  ellipse(origin, b.body.position + float2{2.8f, 4.0f}, b.body.radius * 0.96f, color(0.02f, 0.025f, 0.025f, 0.20f));
  ellipse(origin, b.body.position, b.body.radius, b.fill);
  if (b.striped) {
    brush::color(color(0.96f, 0.94f, 0.84f));
    if (auto res = fill_rectangle(origin + b.body.position + float2{-b.body.radius, -b.body.radius * 0.32f}, {b.body.radius * 2.0f, b.body.radius * 0.64f}); !res)
      res.error().go_off();
  }
  ellipse(origin, b.body.position + float2{-4.4f, -5.0f}, b.body.radius * 0.24f, color(1.0f, 1.0f, 1.0f, 0.28f));

  if (number > 0) {
    ellipse(origin, b.body.position, b.body.radius * 0.45f, color(0.97f, 0.94f, 0.84f));
    text label(format<wchar_t>(number), {.size = 9.0f});
    const auto s = label.size();
    if (auto res = draw_text(origin + b.body.position - s * 0.5f + float2{0.0f, -0.8f}, label, color(0.04f, 0.05f, 0.05f)); !res)
      res.error().go_off();
  }
}

void draw_table_background(bitmap& target, const table_config& cfg) {
  const float2 origin{};
  if (auto d = target.begin_draw(color(0.11f, 0.12f, 0.13f))) {
    fill_rect(origin, {}, {cfg.table_width, cfg.table_height}, color(0.35f, 0.18f, 0.10f));
    fill_rect(origin, {cfg.rail, cfg.rail}, {cfg.table_width - cfg.rail * 2.0f, cfg.table_height - cfg.rail * 2.0f}, color(0.05f, 0.36f, 0.28f));
    stroke_rect(origin, {}, {cfg.table_width, cfg.table_height}, color(0.70f, 0.48f, 0.24f), 3.0f * cfg.scale);
    stroke_rect(origin, {cfg.rail, cfg.rail}, {cfg.table_width - cfg.rail * 2.0f, cfg.table_height - cfg.rail * 2.0f}, color(0.03f, 0.23f, 0.18f), 2.0f * cfg.scale);

    for (const auto p : pocket_centers(cfg)) ellipse(origin, p, cfg.pocket_radius, color(0.015f, 0.018f, 0.018f));

    line(origin, {cfg.table_width * 0.27f, cfg.rail + 18.0f * cfg.scale}, {cfg.table_width * 0.27f, cfg.table_height - cfg.rail - 18.0f * cfg.scale},
         color(0.88f, 0.84f, 0.68f, 0.18f), cfg.scale);

    if (auto res = d->close(); !res) res.error().go_off();
  } else d.error().go_off();
}

void draw_table_foreground(bitmap& target, const game_state& game) {
  const float2 origin{};
  if (auto d = target.begin_draw(colors::transparent)) {
    for (size_t i = 0; i < game.balls.size(); ++i) draw_ball(origin, game.balls[i], int(i));

    const auto& cue = game.balls.front();
    if (game.aiming) {
      auto dir = cue.body.position - game.pointer;
      const auto len = dir.length();
      if (len > 1.0f) dir /= len;
      const auto pull = yw::min(len, 128.0f * game.cfg.scale);
      line(origin, cue.body.position - dir * 18.0f * game.cfg.scale, cue.body.position - dir * (90.0f * game.cfg.scale + pull * 0.55f), color(0.86f, 0.66f, 0.38f), 4.0f * game.cfg.scale);
      line(origin, cue.body.position, cue.body.position + dir * 180.0f * game.cfg.scale, color(0.95f, 0.94f, 0.80f, 0.35f), 1.4f * game.cfg.scale);
    }
    if (auto res = d->close(); !res) res.error().go_off();
  } else d.error().go_off();
}

void draw_game(ui::blank& canvas, const bitmap& background, const bitmap& foreground) {
  if (auto res = draw_bitmap(canvas.pos(), background); !res) res.error().go_off();
  if (auto res = draw_bitmap(canvas.pos(), foreground); !res) res.error().go_off();
}

void update_phase_controls(
  const game_state& game, ui::label& message, ui::button& continue_button, ui::button& new_game_button) {
  const auto show_choice = game.phase != play_phase::playing;
  message.visible(show_choice);
  continue_button.visible(game.phase == play_phase::cue_pocketed);
  continue_button.enabled(game.phase == play_phase::cue_pocketed);
  new_game_button.visible(show_choice);
  new_game_button.enabled(show_choice);

  if (game.phase == play_phase::cue_pocketed) message.string(L"Cue ball pocketed");
  else if (game.phase == play_phase::clear) message.string(L"Clear");
  else message.string(L"");
}

} // namespace

int main(int argc, char** argv) {
  argument::description = "2D billiards sample";
  auto scale_arg = argument::option<float>("--scale")
                     .alias("-s")
                     .default_value(1.0f)
                     .description("Table size scale. Default: 1.0");
  auto balls_arg = argument::option<int>("--balls")
                     .alias("-b")
                     .default_value(16)
                     .description("Total ball count including the cue ball. Default: 16");
  if (auto res = argument::parse(argc, argv); !res) res.error().go_off();

  const auto cfg = make_config(scale_arg.value(), balls_arg.value());

  auto win = window({.title = L"physics billiards", .resizable = false});
  win.background_color(color(0.11f, 0.12f, 0.13f));

  auto root = ui::grid_layout(win);
  if (auto res = root.columns(ui::free, ui::free, ui::free); !res) res.error().go_off();
  if (auto res = root.rows(38.0f, ui::fit, 42.0f); !res) res.error().go_off();
  root.gap({8.0f, 8.0f}).padding(float4::fill(12.0f));

  auto shots = ui::label(root.at({0, 0}));
  auto status = ui::label(root.at({1, 0}));
  auto hint = ui::label(root.at({2, 0}));
  auto canvas = ui::blank(root.at({0, 1}, {3, 1}));
  auto phase_message = ui::label(root.at({0, 2}));
  auto continue_button = ui::button(root.at({1, 2}));
  auto new_game_button = ui::button(root.at({2, 2}));
  canvas.size({cfg.table_width, cfg.table_height});

  shots.text_align(ui::left).background_color(colors::transparent).text_color(color(0.90f, 0.93f, 0.94f));
  status.background_color(colors::transparent).text_color(color(0.90f, 0.93f, 0.94f));
  hint.text_align(ui::right).background_color(colors::transparent).text_color(color(0.74f, 0.80f, 0.80f));
  phase_message.text_align(ui::left).background_color(colors::transparent).text_color(color(0.94f, 0.91f, 0.78f));
  continue_button.string(L"Continue")
    .background_color(color(0.16f, 0.35f, 0.28f))
    .text_color(color(0.93f, 0.96f, 0.92f));
  new_game_button.string(L"New Game")
    .background_color(color(0.26f, 0.23f, 0.20f))
    .text_color(color(0.94f, 0.91f, 0.82f));

  game_state game;
  game.cfg = cfg;
  input_state input;
  game.rack();
  update_phase_controls(game, phase_message, continue_button, new_game_button);

  auto background = bitmap(cfg.bitmap_size());
  auto foreground = bitmap(cfg.bitmap_size());
  draw_table_background(background, cfg);

  win.pointer_event([&](pointer_event e) {
    game.pointer = to_table(canvas, e.pos);
    return true;
  });
  win.button_event([&](button_event e) {
    if (e.key == keys::lbutton && !e.down) input.shoot = true;
    return true;
  });
  win.key_event([&](key_event e) {
    if (e.key == keys::r && e.down) return input.reset = true, true;
    if (e.key == keys::space && e.down) return input.shoot = true, true;
    return false;
  });
  continue_button.click_event([&](button_event) {
    input.continue_game = true;
    return true;
  });
  new_game_button.click_event([&](button_event) {
    input.new_game = true;
    return true;
  });

  while (mainloop) {
    const auto dt = float(yw::min(mainloop.spf(), 1.0 / 30.0));
    update_game(game, input, dt);

    const auto remaining = remaining_object_balls(game);
    shots.string(format<wchar_t>(L"Shots ", game.shots));
    if (game.phase == play_phase::cue_pocketed) status.string(L"Cue pocketed");
    else if (game.phase == play_phase::clear) status.string(L"Clear");
    else status.string(game.moving() ? L"Rolling" : format<wchar_t>(L"Balls ", yw::max(remaining, 0)));
    hint.string(game.phase == play_phase::playing ? L"Mouse aim / click or Space / R" : L"Choose below");
    update_phase_controls(game, phase_message, continue_button, new_game_button);
    draw_table_foreground(foreground, game);

    if (auto d = win.begin_draw()) {
      draw_game(canvas, background, foreground);
    } else d.error().go_off();
  }
}
