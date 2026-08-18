#include <ywxlib>

using namespace yw;

namespace {

constexpr float table_width = 860.0f;
constexpr float table_height = 500.0f;
constexpr float rail = 34.0f;
constexpr float ball_radius = 13.0f;
constexpr float pocket_radius = 28.0f;
constexpr float cue_power = 720.0f;
constexpr float felt_drag = 0.48f;
constexpr float stop_speed = 6.0f;
constexpr uint2 canvas_size{916, 556};

struct ball {
  phy::circle_body<float> body;
  color fill;
  bool striped = false;
  bool active = true;
};

struct game_state {
  std::vector<ball> balls;
  float2 pointer{};
  bool aiming = false;
  int shots = 0;

  phy::aabb<float> bounds() const noexcept {
    return {{rail + ball_radius, rail + ball_radius}, {table_width - rail - ball_radius, table_height - rail - ball_radius}};
  }

  bool moving() const {
    return std::ranges::any_of(balls, [](const ball& b) { return b.active && b.body.velocity.squared_length() > stop_speed * stop_speed; });
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

    balls.push_back({.body = {.position = {table_width * 0.27f, table_height * 0.5f}, .radius = ball_radius, .mass = 1.0f}, .fill = colors[0]});

    const auto start = float2{table_width * 0.67f, table_height * 0.5f};
    int number = 1;
    for (int row = 0; row < 5; ++row) {
      for (int col = 0; col <= row; ++col) {
        const auto x = start.x + float(row) * ball_radius * 1.76f;
        const auto y = start.y + (float(col) - float(row) * 0.5f) * ball_radius * 2.08f;
        balls.push_back({
          .body = {.position = {x, y}, .radius = ball_radius, .mass = 1.0f},
          .fill = colors[(number - 1) % 8 + 1],
          .striped = number > 8,
        });
        ++number;
      }
    }
    shots = 0;
    aiming = false;
  }
};

struct input_state {
  bool shoot = false;
  bool reset = false;
};

float2 table_origin() {
  return (float2{float(canvas_size.x), float(canvas_size.y)} - float2{table_width, table_height}) * 0.5f;
}

float2 to_table(ui::blank& canvas, short2 p) {
  return float2{float(p.x), float(p.y)} - canvas.pos() - table_origin();
}

std::array<float2, 6> pocket_centers() {
  return {
    float2{rail, rail},
    float2{table_width * 0.5f, rail * 0.82f},
    float2{table_width - rail, rail},
    float2{rail, table_height - rail},
    float2{table_width * 0.5f, table_height - rail * 0.82f},
    float2{table_width - rail, table_height - rail},
  };
}

void sink_pocketed_balls(game_state& game) {
  for (auto& b : game.balls) {
    if (!b.active) continue;
    for (const auto p : pocket_centers()) {
      if ((b.body.position - p).length() > pocket_radius * 0.82f) continue;
      if (&b == &game.balls.front()) {
        b.body.position = {table_width * 0.27f, table_height * 0.5f};
        b.body.velocity = {};
      } else {
        b.active = false;
        b.body.velocity = {};
      }
      break;
    }
  }
}

void update_game(game_state& game, input_state& input, float dt) {
  if (input.reset) {
    game.rack();
    input.reset = false;
    input.shoot = false;
  }

  const auto any_moving = game.moving();
  auto& cue = game.balls.front();
  game.aiming = !any_moving && cue.active;

  if (input.shoot && game.aiming) {
    auto dir = cue.body.position - game.pointer;
    const auto len = dir.length();
    if (len > 14.0f) {
      dir /= len;
      const auto pull = yw::min(len, 128.0f);
      cue.body.velocity = dir * (cue_power * pull / 128.0f);
      ++game.shots;
    }
  }
  input.shoot = false;

  const auto bounds = game.bounds();
  for (auto& b : game.balls) {
    if (!b.active) continue;
    b.body.position = phy::integrate(b.body.position, b.body.velocity, dt);
    b.body.velocity = phy::damp(b.body.velocity, felt_drag, dt);
    if (b.body.velocity.length() < stop_speed) b.body.velocity = {};
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
    if (auto res = fill_rectangle(origin + b.body.position + float2{-ball_radius, -4.2f}, {ball_radius * 2.0f, 8.4f}); !res)
      res.error().go_off();
  }
  ellipse(origin, b.body.position + float2{-4.4f, -5.0f}, b.body.radius * 0.24f, color(1.0f, 1.0f, 1.0f, 0.28f));

  if (number > 0) {
    ellipse(origin, b.body.position, 5.8f, color(0.97f, 0.94f, 0.84f));
    text label(format<wchar_t>(number), {.size = 9.0f});
    label.color(color(0.04f, 0.05f, 0.05f));
    const auto s = label.size();
    if (auto res = draw_text(origin + b.body.position - s * 0.5f + float2{0.0f, -0.8f}, label); !res) res.error().go_off();
  }
}

void draw_table_background(bitmap& target) {
  const auto origin = table_origin();
  if (auto d = target.begin_draw(color(0.11f, 0.12f, 0.13f))) {
    fill_rect(origin, {}, {table_width, table_height}, color(0.35f, 0.18f, 0.10f));
    fill_rect(origin, {rail, rail}, {table_width - rail * 2.0f, table_height - rail * 2.0f}, color(0.05f, 0.36f, 0.28f));
    stroke_rect(origin, {}, {table_width, table_height}, color(0.70f, 0.48f, 0.24f), 3.0f);
    stroke_rect(origin, {rail, rail}, {table_width - rail * 2.0f, table_height - rail * 2.0f}, color(0.03f, 0.23f, 0.18f), 2.0f);

    for (const auto p : pocket_centers()) ellipse(origin, p, pocket_radius, color(0.015f, 0.018f, 0.018f));

    line(origin, {table_width * 0.27f, rail + 18.0f}, {table_width * 0.27f, table_height - rail - 18.0f},
         color(0.88f, 0.84f, 0.68f, 0.18f));

    if (auto res = d->close(); !res) res.error().go_off();
  } else d.error().go_off();
}

void draw_table_foreground(bitmap& target, const game_state& game) {
  const auto origin = table_origin();
  if (auto d = target.begin_draw(colors::transparent)) {
  for (size_t i = 0; i < game.balls.size(); ++i) draw_ball(origin, game.balls[i], int(i));

  const auto& cue = game.balls.front();
  if (game.aiming) {
    auto dir = cue.body.position - game.pointer;
    const auto len = dir.length();
    if (len > 1.0f) dir /= len;
    const auto pull = yw::min(len, 128.0f);
    line(origin, cue.body.position - dir * 18.0f, cue.body.position - dir * (90.0f + pull * 0.55f), color(0.86f, 0.66f, 0.38f), 4.0f);
    line(origin, cue.body.position, cue.body.position + dir * 180.0f, color(0.95f, 0.94f, 0.80f, 0.35f), 1.4f);
  }
    if (auto res = d->close(); !res) res.error().go_off();
  } else d.error().go_off();
}

void draw_game(ui::blank& canvas, const bitmap& background, const bitmap& foreground) {
  if (auto res = draw_bitmap(canvas.pos(), background); !res) res.error().go_off();
  if (auto res = draw_bitmap(canvas.pos(), foreground); !res) res.error().go_off();
}

} // namespace

int main() {
  auto win = window({.title = L"physics billiards", .resizable = false});
  win.background_color(color(0.11f, 0.12f, 0.13f));

  auto root = ui::grid_layout(win);
  if (auto res = root.columns(ui::free, ui::free, ui::free); !res) res.error().go_off();
  if (auto res = root.rows(38.0f, table_height + 56.0f); !res) res.error().go_off();
  root.gap({8.0f, 8.0f}).padding(float4::fill(12.0f));

  auto shots = ui::label(root.at({0, 0}));
  auto status = ui::label(root.at({1, 0}));
  auto hint = ui::label(root.at({2, 0}));
  auto canvas = ui::blank(root.at({0, 1}, {3, 1}));
  canvas.size({float(canvas_size.x), float(canvas_size.y)});

  shots.text_align(ui::left).background_color(colors::transparent).text_color(color(0.90f, 0.93f, 0.94f));
  status.background_color(colors::transparent).text_color(color(0.90f, 0.93f, 0.94f));
  hint.text_align(ui::right).background_color(colors::transparent).text_color(color(0.74f, 0.80f, 0.80f));

  game_state game;
  input_state input;
  game.rack();

  auto background = bitmap(canvas_size);
  auto foreground = bitmap(canvas_size);
  draw_table_background(background);

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

  while (mainloop) {
    const auto dt = float(yw::min(mainloop.spf(), 1.0 / 30.0));
    update_game(game, input, dt);

    const auto remaining = int(std::ranges::count_if(game.balls, [](const ball& b) { return b.active; })) - 1;
    shots.string(format<wchar_t>(L"Shots ", game.shots));
    status.string(game.moving() ? L"Rolling" : format<wchar_t>(L"Balls ", yw::max(remaining, 0)));
    hint.string(L"Mouse aim / click or Space / R");
    draw_table_foreground(foreground, game);

    if (auto d = win.begin_draw()) {
      draw_game(canvas, background, foreground);
    } else d.error().go_off();
  }
}
