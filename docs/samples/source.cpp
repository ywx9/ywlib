#include <ywxlib>

using namespace yw;

namespace {

constexpr float board_width = 620.0f;
constexpr float board_height = 760.0f;
constexpr float ball_radius = 9.0f;
constexpr float wall_radius = 8.0f;
constexpr float flipper_radius = 11.0f;
constexpr float flipper_length = 104.0f;
constexpr float gravity = 520.0f;
constexpr float max_speed = 980.0f;
constexpr float launch_x = 562.0f;
constexpr int start_balls = 3;

struct bumper {
  float2 pos;
  float radius;
  int score;
  float flash = 0.0f;
};

struct flipper {
  float2 pivot;
  float rest_angle;
  float active_angle;
  float angle;
  float angular_velocity = 0.0f;

  phy::capsule<float> capsule() const noexcept {
    const auto tip = pivot + float2{yw::cos(angle), yw::sin(angle)} * flipper_length;
    return {{pivot, tip}, flipper_radius};
  }

  void reset() noexcept {
    angle = rest_angle;
    angular_velocity = 0.0f;
  }

  void update(bool active, float dt) noexcept {
    const auto old = angle;
    const auto target = active ? active_angle : rest_angle;
    angle += (target - angle) * yw::min(1.0f, 18.0f * dt);
    angular_velocity = dt > 0.0f ? (angle - old) / dt : 0.0f;
  }

  float2 surface_velocity(float2 point) const noexcept {
    return phy::perp(point - pivot) * angular_velocity;
  }
};

struct input_state {
  bool left = false;
  bool right = false;
  bool plunger = false;
  bool plunger_released = false;
  bool reset = false;

  bool set(key k, bool down) {
    if (k == keys::left || k == keys::a) return left = down, true;
    if (k == keys::right || k == keys::d) return right = down, true;
    if (k == keys::space || k == keys::down || k == keys::s) {
      if (plunger && !down) plunger_released = true;
      plunger = down;
      return true;
    }
    if (k == keys::r && down) return reset = true, true;
    return false;
  }
};

struct game_state {
  phy::circle_body<float> ball;
  std::array<bumper, 4> bumpers;
  flipper left_flipper;
  flipper right_flipper;
  float plunger = 0.0f;
  double elapsed = 0.0;
  int score = 0;
  int balls = start_balls;
  bool waiting = true;
  bool game_over = false;

  void serve() {
    ball = {.position = {launch_x, board_height - 80.0f}, .velocity = {}, .radius = ball_radius, .mass = 1.0f};
    plunger = 0.0f;
    waiting = true;
  }

  void restart() {
    score = 0;
    balls = start_balls;
    elapsed = 0.0;
    game_over = false;
    bumpers = {
      bumper{{240.0f, 178.0f}, 31.0f, 100},
      bumper{{365.0f, 215.0f}, 34.0f, 100},
      bumper{{205.0f, 320.0f}, 28.0f, 150},
      bumper{{410.0f, 365.0f}, 28.0f, 150},
    };
    left_flipper = {{214.0f, 646.0f}, 0.42f, -0.54f, 0.42f};
    right_flipper = {{406.0f, 646.0f}, float(yw::pi) - 0.42f, float(yw::pi) + 0.54f, float(yw::pi) - 0.42f};
    left_flipper.reset();
    right_flipper.reset();
    serve();
  }
};

std::array<phy::capsule<float>, 13> table_walls() {
  return {
    phy::capsule<float>{{{63.0f, 690.0f}, {63.0f, 134.0f}}, wall_radius},
    phy::capsule<float>{{{63.0f, 134.0f}, {146.0f, 50.0f}}, wall_radius},
    phy::capsule<float>{{{146.0f, 50.0f}, {488.0f, 50.0f}}, wall_radius},
    phy::capsule<float>{{{488.0f, 50.0f}, {548.0f, 122.0f}}, wall_radius},
    phy::capsule<float>{{{548.0f, 122.0f}, {548.0f, 690.0f}}, wall_radius},
    phy::capsule<float>{{{63.0f, 690.0f}, {168.0f, 610.0f}}, wall_radius},
    phy::capsule<float>{{{548.0f, 690.0f}, {452.0f, 610.0f}}, wall_radius},
    phy::capsule<float>{{{148.0f, 510.0f}, {246.0f, 562.0f}}, wall_radius},
    phy::capsule<float>{{{472.0f, 510.0f}, {374.0f, 562.0f}}, wall_radius},
    phy::capsule<float>{{{515.0f, 128.0f}, {515.0f, 706.0f}}, 5.0f},
    phy::capsule<float>{{{548.0f, 706.0f}, {594.0f, 706.0f}}, wall_radius},
    phy::capsule<float>{{{594.0f, 706.0f}, {594.0f, 108.0f}}, wall_radius},
    phy::capsule<float>{{{548.0f, 88.0f}, {594.0f, 120.0f}}, wall_radius},
  };
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

void stroke_ellipse_at(float2 origin, float2 center, float radius, const color& c, float width = 1.0f) {
  brush::color(c);
  if (auto res = stroke_ellipse(origin + center, float2::fill(radius), width); !res) res.error().go_off();
}

void draw_capsule(float2 origin, const phy::capsule<float>& cap, const color& c) {
  line(origin, cap.line.a, cap.line.b, c, cap.radius * 2.0f);
  ellipse(origin, cap.line.a, cap.radius, c);
  ellipse(origin, cap.line.b, cap.radius, c);
}

void collide_bumper(game_state& game, bumper& b) {
  const phy::circle<float> shape{b.pos, b.radius};
  const auto contact = phy::circle_contact(shape, phy::circle<float>{game.ball.position, game.ball.radius});
  if (contact.penetration <= 0.0f) return;

  const auto approaching = dot(game.ball.velocity, contact.normal) < 0.0f;
  phy::resolve_static_collision(game.ball, contact, {}, 1.18f);
  if (approaching) {
    game.ball.velocity += contact.normal * 92.0f;
    game.score += b.score;
    b.flash = 0.12f;
  }
}

void collide_flipper(game_state& game, const flipper& f) {
  const auto cap = f.capsule();
  const auto nearest = phy::closest_point(game.ball.position, cap.line);
  phy::resolve_collision(game.ball, cap, f.surface_velocity(nearest), 1.05f);
}

void update_game(game_state& game, input_state& input, float dt) {
  if (input.reset) {
    game.restart();
    input.reset = false;
    input.plunger_released = false;
    return;
  }

  game.elapsed += dt;
  game.left_flipper.update(input.left, dt);
  game.right_flipper.update(input.right, dt);
  for (auto& b : game.bumpers) b.flash = yw::max(0.0f, b.flash - dt);

  if (game.game_over) {
    if (input.plunger_released) game.restart();
    input.plunger_released = false;
    return;
  }

  if (game.waiting) {
    game.ball.position = {launch_x, board_height - 80.0f - game.plunger * 36.0f};
    game.ball.velocity = {};
    if (input.plunger) game.plunger = yw::min(1.0f, game.plunger + dt * 1.25f);
    if (input.plunger_released) {
      game.ball.velocity = {0.0f, -560.0f - game.plunger * 560.0f};
      game.waiting = false;
      game.plunger = 0.0f;
    }
    input.plunger_released = false;
    return;
  }

  game.ball.velocity.y += gravity * dt;
  game.ball.velocity = phy::damp(game.ball.velocity, 0.998f, dt * 60.0f);
  if (const auto speed = game.ball.velocity.length(); speed > max_speed) game.ball.velocity *= max_speed / speed;
  game.ball.position = phy::integrate(game.ball.position, game.ball.velocity, dt);

  for (int pass = 0; pass < 3; ++pass) {
    for (const auto& wall : table_walls()) phy::resolve_collision(game.ball, wall, {}, 0.86f);
    for (auto& b : game.bumpers) collide_bumper(game, b);
    collide_flipper(game, game.left_flipper);
    collide_flipper(game, game.right_flipper);
  }

  if (game.ball.position.y - game.ball.radius > board_height + 24.0f) {
    --game.balls;
    if (game.balls <= 0) game.game_over = true;
    else game.serve();
  }
}

void draw_table(ui::blank& canvas, const game_state& game, const input_state& input) {
  if (const auto& g = canvas.geometry()) d2d::push_layer(g.get());

  const auto origin = canvas.pos();
  const auto size = canvas.size();
  fill_rect(origin, {}, size, color(0.045f, 0.050f, 0.060f));
  stroke_rect(origin, {}, size, color(0.28f, 0.30f, 0.34f), 1.0f);

  fill_rect(origin, {28.0f, 28.0f}, {564.0f, 704.0f}, color(0.08f, 0.095f, 0.12f));

  for (int y = 0; y < 11; ++y) {
    const auto yy = 80.0f + float(y) * 58.0f;
    line(origin, {82.0f, yy}, {498.0f, yy + 20.0f}, color(0.12f, 0.14f, 0.17f), 1.0f);
  }

  for (const auto& wall : table_walls()) draw_capsule(origin, wall, color(0.58f, 0.32f, 0.20f));

  stroke_ellipse_at(origin, {310.0f, 198.0f}, 132.0f, color(0.26f, 0.34f, 0.42f), 2.0f);
  line(origin, {94.0f, 704.0f}, {260.0f, 704.0f}, color(0.19f, 0.21f, 0.24f), 3.0f);
  line(origin, {360.0f, 704.0f}, {516.0f, 704.0f}, color(0.19f, 0.21f, 0.24f), 3.0f);

  for (const auto& b : game.bumpers) {
    const auto glow = b.flash > 0.0f ? color(1.0f, 0.86f, 0.34f, 0.42f) : color(0.30f, 0.40f, 0.50f, 0.28f);
    ellipse(origin, b.pos, b.radius + 9.0f, glow);
    ellipse(origin, b.pos, b.radius, color(0.92f, 0.24f, 0.28f));
    ellipse(origin, b.pos + float2{-7.0f, -8.0f}, b.radius * 0.26f, color(1.0f, 0.92f, 0.70f, 0.55f));
    stroke_ellipse_at(origin, b.pos, b.radius, color(0.98f, 0.74f, 0.38f), 2.0f);
  }

  draw_capsule(origin, game.left_flipper.capsule(), input.left ? color(0.98f, 0.78f, 0.34f) : color(0.86f, 0.58f, 0.26f));
  draw_capsule(origin, game.right_flipper.capsule(), input.right ? color(0.98f, 0.78f, 0.34f) : color(0.86f, 0.58f, 0.26f));

  const auto plunger_y = board_height - 48.0f + game.plunger * 30.0f;
  line(origin, {launch_x, board_height - 42.0f}, {launch_x, plunger_y}, color(0.72f, 0.76f, 0.80f), 7.0f);
  ellipse(origin, {launch_x, plunger_y}, 12.0f, color(0.80f, 0.30f, 0.25f));

  ellipse(origin, game.ball.position + float2{3.4f, 5.0f}, game.ball.radius * 0.96f, color(0.0f, 0.0f, 0.0f, 0.28f));
  ellipse(origin, game.ball.position, game.ball.radius, color(0.90f, 0.94f, 0.96f));
  ellipse(origin, game.ball.position + float2{-3.0f, -4.0f}, game.ball.radius * 0.30f, color(1.0f, 1.0f, 1.0f, 0.68f));

  if (game.waiting || game.game_over) {
    text prompt(game.game_over ? L"Game over - Space: new game" : L"Hold Space, release to launch", {.size = 25.0f});
    const auto p = origin + float2{(board_width - prompt.size().x) * 0.5f, 418.0f};
    if (auto res = draw_text(p + float2{2.0f, 2.0f}, prompt, color(0.0f, 0.0f, 0.0f, 0.40f)); !res) res.error().go_off();
    if (auto res = draw_text(p, prompt, color(0.94f, 0.96f, 0.92f)); !res) res.error().go_off();
  }

  if (canvas.geometry()) d2d::pop_layer();
}

} // namespace

int main() {
  auto win = window({.title = L"2D pinball", .resizable = false});
  win.background_color(color(0.10f, 0.11f, 0.13f));

  auto root = ui::grid_layout(win);
  if (auto res = root.columns(ui::free, ui::free, ui::free); !res) res.error().go_off();
  if (auto res = root.rows(38.0f, board_height); !res) res.error().go_off();
  root.gap({8.0f, 10.0f}).padding(float4::fill(12.0f));

  auto score = ui::label(root.at({0, 0}));
  auto status = ui::label(root.at({1, 0}));
  auto balls = ui::label(root.at({2, 0}));
  auto canvas = ui::blank(root.at({0, 1}, {3, 1}));
  canvas.size({board_width, board_height});

  score.text_align(ui::left).background_color(colors::transparent).text_color(color(0.92f, 0.94f, 0.96f));
  status.background_color(colors::transparent).text_color(color(0.92f, 0.94f, 0.96f));
  balls.text_align(ui::right).background_color(colors::transparent).text_color(color(0.92f, 0.94f, 0.96f));

  game_state game;
  input_state input;
  game.restart();

  win.key_event([&](key_event e) {
    return input.set(e.key, e.down);
  });

  while (mainloop) {
    const auto dt = float(yw::min(mainloop.spf(), 1.0 / 45.0));
    update_game(game, input, dt);

    score.string(format<wchar_t>(L"Score ", game.score));
    status.string(game.game_over ? L"Game over" : (game.waiting ? L"Launch" : L"Playing"));
    balls.string(format<wchar_t>(L"Balls ", yw::max(game.balls, 0), L"   A/Left  D/Right  Space  R"));

    if (auto d = win.begin_draw()) {
      draw_table(canvas, game, input);
    } else d.error().go_off();
  }
}
