#include <ywxlib>

using namespace yw;

namespace {

constexpr float board_width = 760.0f;
constexpr float board_height = 560.0f;
constexpr float ship_radius = 17.0f;
constexpr float ship_turn_speed = 4.4f;
constexpr float ship_thrust = 360.0f;
constexpr float ship_drag = 0.992f;
constexpr float shot_speed = 520.0f;
constexpr float shot_life = 0.88f;
constexpr float shot_cooldown = 0.18f;
constexpr int start_lives = 3;

struct shot {
  float2 pos;
  float2 vel;
  float life = shot_life;
};

struct asteroid {
  float2 pos;
  float2 vel;
  float radius = 44.0f;
  float angle = 0.0f;
  float spin = 0.0f;
  int size = 3;
  std::array<float, 10> shape{};
};

struct particle {
  float2 pos;
  float2 vel;
  float life = 0.0f;
};

struct input_state {
  bool left = false;
  bool right = false;
  bool thrust = false;
  bool fire = false;

  bool set(key k, bool down) {
    if (k == keys::left || k == keys::a) return left = down, true;
    if (k == keys::right || k == keys::d) return right = down, true;
    if (k == keys::up || k == keys::w) return thrust = down, true;
    if (k == keys::space) {
      if (down) fire = true;
      return true;
    }
    return false;
  }
};

float2 heading(float angle) {
  return {yw::cos(angle), yw::sin(angle)};
}

float2 random_velocity(float min_speed, float max_speed) {
  const auto angle = random::gen<float>(0.0f, float(yw::pi * 2.0));
  return heading(angle) * random::gen<float>(min_speed, max_speed);
}

float2 wrap_pos(float2 p) {
  if (p.x < 0.0f) p.x += board_width;
  if (p.x >= board_width) p.x -= board_width;
  if (p.y < 0.0f) p.y += board_height;
  if (p.y >= board_height) p.y -= board_height;
  return p;
}

float2 torus_delta(float2 a, float2 b) {
  auto d = a - b;
  if (d.x > board_width * 0.5f) d.x -= board_width;
  if (d.x < -board_width * 0.5f) d.x += board_width;
  if (d.y > board_height * 0.5f) d.y -= board_height;
  if (d.y < -board_height * 0.5f) d.y += board_height;
  return d;
}

asteroid make_asteroid(float2 pos, int size) {
  asteroid a;
  a.pos = pos;
  a.size = size;
  a.radius = size == 3 ? 48.0f : (size == 2 ? 31.0f : 18.0f);
  a.vel = random_velocity(size == 3 ? 32.0f : 56.0f, size == 3 ? 86.0f : 142.0f);
  a.angle = random::gen<float>(0.0f, float(yw::pi * 2.0));
  a.spin = random::gen<float>(-1.4f, 1.4f);
  for (auto& r : a.shape) r = random::gen<float>(0.72f, 1.18f);
  return a;
}

struct game_state {
  std::vector<asteroid> asteroids;
  std::vector<shot> shots;
  std::vector<particle> particles;
  float2 ship_pos = {board_width * 0.5f, board_height * 0.5f};
  float2 ship_vel{};
  float ship_angle = -float(yw::pi) * 0.5f;
  float cooldown = 0.0f;
  float invincible = 0.0f;
  double elapsed = 0.0;
  int score = 0;
  int lives = start_lives;
  int wave = 1;
  bool running = false;
  bool game_over = false;

  void spawn_wave() {
    asteroids.clear();
    const int count = 3 + wave;
    for (int i = 0; i < count; ++i) {
      float2 pos;
      do {
        pos = {random::gen<float>(42.0f, board_width - 42.0f), random::gen<float>(42.0f, board_height - 42.0f)};
      } while ((pos - ship_pos).length() < 160.0f);
      asteroids.push_back(make_asteroid(pos, 3));
    }
  }

  void reset_ship() {
    ship_pos = {board_width * 0.5f, board_height * 0.5f};
    ship_vel = {};
    ship_angle = -float(yw::pi) * 0.5f;
    cooldown = 0.0f;
    invincible = 2.0f;
  }

  void restart() {
    shots.clear();
    particles.clear();
    score = 0;
    lives = start_lives;
    wave = 1;
    elapsed = 0.0;
    running = false;
    game_over = false;
    reset_ship();
    spawn_wave();
  }
};

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

float2 rotated(float2 p, float angle) {
  const auto c = yw::cos(angle);
  const auto s = yw::sin(angle);
  return {p.x * c - p.y * s, p.x * s + p.y * c};
}

void draw_polyline_wrapped(float2 origin, std::span<const float2> points, const color& c, float width) {
  brush::color(c);
  for (int ox = -1; ox <= 1; ++ox) {
    for (int oy = -1; oy <= 1; ++oy) {
      const auto offset = float2{float(ox) * board_width, float(oy) * board_height};
      for (size_t i = 0; i < points.size(); ++i) {
        if (auto res = stroke_line(origin + points[i] + offset, origin + points[(i + 1) % points.size()] + offset, width); !res)
          res.error().go_off();
      }
    }
  }
}

void draw_ship(float2 origin, const game_state& game, bool thrusting) {
  if (game.invincible > 0.0f && int(game.elapsed * 12.0) % 2 == 0) return;

  const std::array<float2, 4> local = {
    float2{22.0f, 0.0f},
    float2{-15.0f, -13.0f},
    float2{-8.0f, 0.0f},
    float2{-15.0f, 13.0f},
  };
  std::array<float2, local.size()> pts;
  for (size_t i = 0; i < local.size(); ++i) pts[i] = game.ship_pos + rotated(local[i], game.ship_angle);
  draw_polyline_wrapped(origin, pts, color(0.88f, 0.95f, 1.0f), 2.0f);

  if (thrusting && game.running) {
    const auto tail = game.ship_pos + rotated({-16.0f, 0.0f}, game.ship_angle);
    const auto flame = game.ship_pos + rotated({-29.0f - random::gen<float>(0.0f, 6.0f), 0.0f}, game.ship_angle);
    line(origin, tail, flame, color(1.0f, 0.62f, 0.28f), 2.0f);
  }
}

void draw_asteroid(float2 origin, const asteroid& a) {
  std::array<float2, 10> pts;
  for (size_t i = 0; i < pts.size(); ++i) {
    const auto angle = a.angle + float(i) / float(pts.size()) * float(yw::pi * 2.0);
    pts[i] = a.pos + heading(angle) * (a.radius * a.shape[i]);
  }
  draw_polyline_wrapped(origin, pts, color(0.76f, 0.80f, 0.82f), a.size == 3 ? 2.2f : 1.8f);
}

void emit_burst(game_state& game, float2 pos, int count) {
  for (int i = 0; i < count; ++i) {
    game.particles.push_back({
      .pos = pos,
      .vel = random_velocity(42.0f, 210.0f),
      .life = random::gen<float>(0.28f, 0.72f),
    });
  }
}

void fire(game_state& game) {
  if (!game.running || game.cooldown > 0.0f) return;
  const auto dir = heading(game.ship_angle);
  game.shots.push_back({
    .pos = wrap_pos(game.ship_pos + dir * 24.0f),
    .vel = game.ship_vel + dir * shot_speed,
    .life = shot_life,
  });
  game.cooldown = shot_cooldown;
}

void split_asteroid(game_state& game, const asteroid& a) {
  emit_burst(game, a.pos, a.size == 3 ? 16 : 10);
  game.score += a.size == 3 ? 20 : (a.size == 2 ? 50 : 100);
  if (a.size <= 1) return;

  for (int i = 0; i < 2; ++i) {
    auto child = make_asteroid(wrap_pos(a.pos + random_velocity(4.0f, 18.0f)), a.size - 1);
    child.vel += a.vel.normalized() * 32.0f;
    game.asteroids.push_back(child);
  }
}

void lose_life(game_state& game) {
  emit_burst(game, game.ship_pos, 22);
  --game.lives;
  game.shots.clear();
  if (game.lives <= 0) {
    game.running = false;
    game.game_over = true;
  } else {
    game.running = false;
    game.reset_ship();
  }
}

void update_game(game_state& game, input_state& input, float dt) {
  game.elapsed += dt;
  game.cooldown = yw::max(0.0f, game.cooldown - dt);
  game.invincible = yw::max(0.0f, game.invincible - dt);

  for (auto& p : game.particles) {
    p.pos = wrap_pos(p.pos + p.vel * dt);
    p.vel *= 0.985f;
    p.life -= dt;
  }
  std::erase_if(game.particles, [](const particle& p) { return p.life <= 0.0f; });

  if (input.fire) {
    if (game.game_over) game.restart();
    else if (!game.running) game.running = true;
    else fire(game);
    input.fire = false;
  }

  if (game.running) {
    if (input.left) game.ship_angle -= ship_turn_speed * dt;
    if (input.right) game.ship_angle += ship_turn_speed * dt;
    if (input.thrust) game.ship_vel += heading(game.ship_angle) * (ship_thrust * dt);
    game.ship_vel *= yw::pow(ship_drag, dt * 60.0f);
    game.ship_pos = wrap_pos(game.ship_pos + game.ship_vel * dt);
  }

  for (auto& s : game.shots) {
    s.pos = wrap_pos(s.pos + s.vel * dt);
    s.life -= dt;
  }
  std::erase_if(game.shots, [](const shot& s) { return s.life <= 0.0f; });

  for (auto& a : game.asteroids) {
    a.pos = wrap_pos(a.pos + a.vel * dt);
    a.angle += a.spin * dt;
  }

  for (size_t si = 0; si < game.shots.size();) {
    bool hit = false;
    for (size_t ai = 0; ai < game.asteroids.size(); ++ai) {
      const auto delta = torus_delta(game.shots[si].pos, game.asteroids[ai].pos);
      if (delta.squared_length() > game.asteroids[ai].radius * game.asteroids[ai].radius) continue;
      const auto old = game.asteroids[ai];
      game.shots.erase(game.shots.begin() + ptrdiff_t(si));
      game.asteroids.erase(game.asteroids.begin() + ptrdiff_t(ai));
      split_asteroid(game, old);
      hit = true;
      break;
    }
    if (!hit) ++si;
  }

  if (game.running && game.invincible <= 0.0f) {
    for (const auto& a : game.asteroids) {
      const auto radius = a.radius + ship_radius;
      if (torus_delta(game.ship_pos, a.pos).squared_length() <= radius * radius) {
        lose_life(game);
        break;
      }
    }
  }

  if (game.running && game.asteroids.empty()) {
    ++game.wave;
    game.spawn_wave();
    game.invincible = 1.5f;
  }
}

void draw_game(ui::blank& canvas, const game_state& game, const input_state& input) {
  if (const auto& g = canvas.geometry()) d2d::push_layer(g.get());

  const auto origin = canvas.pos();
  const auto size = canvas.size();

  fill_rect(origin, {}, size, color(0.025f, 0.030f, 0.040f));
  stroke_rect(origin, {}, size, color(0.32f, 0.36f, 0.40f), 1.0f);

  brush::color(color(0.72f, 0.78f, 0.86f, 0.52f));
  for (int i = 0; i < 80; ++i) {
    const float x = yw::fmod(float(i * 97 + 23), board_width);
    const float y = yw::fmod(float(i * 53 + 41), board_height);
    const float r = float((i % 3) + 1);
    if (auto res = fill_ellipse(origin + float2{x, y}, float2::fill(r * 0.55f)); !res) res.error().go_off();
  }

  for (const auto& a : game.asteroids) draw_asteroid(origin, a);

  brush::color(color(0.98f, 0.96f, 0.70f));
  for (const auto& s : game.shots) {
    if (auto res = fill_ellipse(origin + s.pos, float2::fill(2.4f)); !res) res.error().go_off();
  }

  brush::color(color(0.88f, 0.90f, 0.86f, 0.82f));
  for (const auto& p : game.particles) {
    if (auto res = fill_ellipse(origin + p.pos, float2::fill(yw::max(0.8f, p.life * 3.0f))); !res) res.error().go_off();
  }

  draw_ship(origin, game, input.thrust);

  if (!game.running) {
    text prompt(game.game_over ? L"Game over - Space to restart" : L"Space to launch", {.size = 27.0f});
    const auto p = origin + float2{(board_width - prompt.size().x) * 0.5f, board_height * 0.42f};
    if (auto res = draw_text(p, prompt, color(0.92f, 0.96f, 1.0f)); !res) res.error().go_off();
  }

  if (canvas.geometry()) d2d::pop_layer();
}

} // namespace

int main() {
  auto win = window({.title = L"asteroids", .resizable = false});
  win.background_color(color(0.10f, 0.11f, 0.13f));

  auto root = ui::grid_layout(win);
  if (auto res = root.columns(ui::free, ui::free, ui::free); !res) res.error().go_off();
  if (auto res = root.rows(38.0f, board_height); !res) res.error().go_off();
  root.gap({8.0f, 10.0f}).padding(float4::fill(12.0f));

  auto score = ui::label(root.at({0, 0}));
  auto wave = ui::label(root.at({1, 0}));
  auto lives = ui::label(root.at({2, 0}));
  auto canvas = ui::blank(root.at({0, 1}, {3, 1}));
  canvas.size({board_width, board_height});

  score.text_align(ui::left).background_color(colors::transparent).text_color(color(0.88f, 0.92f, 0.96f));
  wave.background_color(colors::transparent).text_color(color(0.88f, 0.92f, 0.96f));
  lives.text_align(ui::right).background_color(colors::transparent).text_color(color(0.88f, 0.92f, 0.96f));

  game_state game;
  input_state input;
  game.restart();

  win.key_event([&](key_event e) {
    return input.set(e.key, e.down);
  });

  while (mainloop) {
    const auto dt = float(yw::min(mainloop.spf(), 1.0 / 30.0));

    update_game(game, input, dt);
    score.string(format<wchar_t>(L"Score ", game.score));
    wave.string(format<wchar_t>(L"Wave ", game.wave));
    lives.string(format<wchar_t>(L"Ships ", yw::max(game.lives, 0)));

    if (auto d = win.begin_draw()) {
      draw_game(canvas, game, input);
    } else d.error().go_off();
  }
}
