#include <ywxlib>

using namespace yw;

namespace {

constexpr float board_width = 520.0f;
constexpr float board_height = 720.0f;
constexpr float ground_height = 86.0f;
constexpr float bird_x = 138.0f;
constexpr float bird_size = 54.0f;
constexpr float gravity = 1320.0f;
constexpr float flap_velocity = -430.0f;
constexpr float pipe_width = 78.0f;
constexpr float pipe_gap = 178.0f;
constexpr float pipe_spacing = 245.0f;
constexpr float pipe_speed = 168.0f;
constexpr float pipe_min_top = 72.0f;
constexpr float pipe_max_top = board_height - ground_height - pipe_gap - 92.0f;
constexpr uint8_t alpha_hit_threshold = 32;

struct pipe {
  float x = 0.0f;
  float gap_y = 0.0f;
  bool scored = false;
};

struct alpha_mask {
  uint2 size{};
  std::vector<uint8_t> opaque;

  bool hit(int x, int y) const {
    if (x < 0 || y < 0 || x >= int(size.x) || y >= int(size.y)) return false;
    return opaque[size_t(y) * size.x + x] != 0;
  }
};

struct assets {
  bitmap_texture birds[3];
  alpha_mask bird_mask;
};

struct game_state {
  std::vector<pipe> pipes;
  float bird_y = board_height * 0.45f;
  float bird_velocity = 0.0f;
  float ground_offset = 0.0f;
  double elapsed = 0.0;
  int score = 0;
  bool running = false;
  bool game_over = false;

  void add_pipe(float x) {
    pipes.push_back({
      .x = x,
      .gap_y = float(random::gen<int>(int(pipe_min_top), int(pipe_max_top))),
      .scored = false,
    });
  }

  void restart() {
    pipes.clear();
    for (int i = 0; i < 4; ++i) add_pipe(board_width + 80.0f + float(i) * pipe_spacing);
    bird_y = board_height * 0.43f;
    bird_velocity = 0.0f;
    ground_offset = 0.0f;
    elapsed = 0.0;
    score = 0;
    running = false;
    game_over = false;
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

void draw_cloud(float2 origin, float2 center, float scale) {
  brush::color(color(1.0f, 1.0f, 1.0f, 0.68f));
  if (auto res = fill_ellipse(origin + center + float2{-22.0f, 4.0f} * scale, float2{24.0f, 13.0f} * scale); !res)
    res.error().go_off();
  if (auto res = fill_ellipse(origin + center + float2{0.0f, -5.0f} * scale, float2{30.0f, 19.0f} * scale); !res)
    res.error().go_off();
  if (auto res = fill_ellipse(origin + center + float2{26.0f, 4.0f} * scale, float2{25.0f, 14.0f} * scale); !res)
    res.error().go_off();
}

void draw_pipe(float2 origin, const pipe& p) {
  const float top_h = p.gap_y;
  const float bottom_y = p.gap_y + pipe_gap;
  const float bottom_h = board_height - ground_height - bottom_y;
  const color pipe_body(0.22f, 0.72f, 0.34f);
  const color pipe_light(0.37f, 0.88f, 0.43f);
  const color pipe_dark(0.08f, 0.39f, 0.18f);

  fill_rect(origin, {p.x, 0.0f}, {pipe_width, top_h}, pipe_body);
  fill_rect(origin, {p.x + 10.0f, 0.0f}, {13.0f, top_h}, pipe_light);
  stroke_rect(origin, {p.x, 0.0f}, {pipe_width, top_h}, pipe_dark, 2.0f);
  fill_rect(origin, {p.x - 8.0f, top_h - 28.0f}, {pipe_width + 16.0f, 28.0f}, pipe_body);
  stroke_rect(origin, {p.x - 8.0f, top_h - 28.0f}, {pipe_width + 16.0f, 28.0f}, pipe_dark, 2.0f);

  fill_rect(origin, {p.x, bottom_y}, {pipe_width, bottom_h}, pipe_body);
  fill_rect(origin, {p.x + 10.0f, bottom_y}, {13.0f, bottom_h}, pipe_light);
  stroke_rect(origin, {p.x, bottom_y}, {pipe_width, bottom_h}, pipe_dark, 2.0f);
  fill_rect(origin, {p.x - 8.0f, bottom_y}, {pipe_width + 16.0f, 28.0f}, pipe_body);
  stroke_rect(origin, {p.x - 8.0f, bottom_y}, {pipe_width + 16.0f, 28.0f}, pipe_dark, 2.0f);
}

bool mask_rect_overlap(const alpha_mask& mask, float2 bird_pos, float2 bird_draw_size, float2 rect_pos, float2 rect_size) {
  const float left = yw::max(bird_pos.x, rect_pos.x);
  const float top = yw::max(bird_pos.y, rect_pos.y);
  const float right = yw::min(bird_pos.x + bird_draw_size.x, rect_pos.x + rect_size.x);
  const float bottom = yw::min(bird_pos.y + bird_draw_size.y, rect_pos.y + rect_size.y);
  if (left >= right || top >= bottom) return false;

  const float sx = float(mask.size.x) / bird_draw_size.x;
  const float sy = float(mask.size.y) / bird_draw_size.y;
  const int x0 = yw::max(0, int(yw::floor((left - bird_pos.x) * sx)));
  const int y0 = yw::max(0, int(yw::floor((top - bird_pos.y) * sy)));
  const int x1 = yw::min(int(mask.size.x), int(yw::ceil((right - bird_pos.x) * sx)));
  const int y1 = yw::min(int(mask.size.y), int(yw::ceil((bottom - bird_pos.y) * sy)));

  for (int y = y0; y < y1; ++y)
    for (int x = x0; x < x1; ++x)
      if (mask.hit(x, y)) return true;
  return false;
}

bool hit_pipe(const game_state& game, const pipe& p, const alpha_mask& mask) {
  const float2 bird_pos{bird_x, game.bird_y};
  const float2 bird_draw_size = float2::fill(bird_size);
  const float bottom_y = p.gap_y + pipe_gap;
  return mask_rect_overlap(mask, bird_pos, bird_draw_size, {p.x, 0.0f}, {pipe_width, p.gap_y}) ||
         mask_rect_overlap(mask, bird_pos, bird_draw_size, {p.x, bottom_y}, {pipe_width, board_height - ground_height - bottom_y});
}

alpha_mask make_alpha_mask(const bitmap_texture& image) {
  std::vector<bgra> pixels(image.size().x * image.size().y);
  if (auto res = image.copy_to_cpu(pixels.data()); !res) res.error().go_off();

  alpha_mask mask;
  mask.size = image.size();
  mask.opaque.resize(pixels.size());
  for (size_t i = 0; i < pixels.size(); ++i) mask.opaque[i] = pixels[i].a > alpha_hit_threshold;
  return mask;
}

assets load_assets() {
  auto archive = archive::handle(L"flappy_bird.ywa", file::open_mode::read_existing);
  assets a;
  const char* names[] = {"bird_a.png", "bird_b.png", "bird_c.png"};
  for (int i = 0; i < 3; ++i) {
    auto data = archive.read(names[i]);
    if (data.empty()) error(errors::operation_failed, "failed to read bird image").go_off();
    a.birds[i] = bitmap_texture(data);
  }
  a.bird_mask = make_alpha_mask(a.birds[1]);
  if (auto res = archive.close(); !res) res.error().go_off();
  return a;
}

void flap(game_state& game) {
  if (game.game_over) game.restart();
  game.running = true;
  game.bird_velocity = flap_velocity;
}

void update_game(game_state& game, const assets& a, float dt) {
  if (!game.running) return;

  game.elapsed += dt;
  game.bird_velocity += gravity * dt;
  game.bird_y += game.bird_velocity * dt;
  game.ground_offset = yw::fmod(game.ground_offset + pipe_speed * dt, 32.0f);

  for (auto& p : game.pipes) {
    p.x -= pipe_speed * dt;
    if (!p.scored && p.x + pipe_width < bird_x) {
      p.scored = true;
      ++game.score;
    }
    if (hit_pipe(game, p, a.bird_mask)) {
      game.running = false;
      game.game_over = true;
    }
  }

  if (!game.pipes.empty() && game.pipes.front().x + pipe_width < -20.0f) {
    const float next_x = game.pipes.back().x + pipe_spacing;
    game.pipes.erase(game.pipes.begin());
    game.add_pipe(next_x);
  }

  if (game.bird_y < -bird_size * 0.5f || game.bird_y + bird_size > board_height - ground_height) {
    game.running = false;
    game.game_over = true;
  }
}

void draw_ground(float2 origin, const game_state& game) {
  fill_rect(origin, {0.0f, board_height - ground_height}, {board_width, ground_height}, color(0.82f, 0.71f, 0.42f));
  fill_rect(origin, {0.0f, board_height - ground_height}, {board_width, 15.0f}, color(0.46f, 0.78f, 0.38f));
  for (float x = -game.ground_offset; x < board_width; x += 32.0f)
    fill_rect(origin, {x, board_height - ground_height + 18.0f}, {18.0f, 5.0f}, color(0.66f, 0.56f, 0.31f));
}

void draw_bird(float2 origin, const game_state& game, const assets& a) {
  int frame = 1;
  if (game.running) frame = int(game.elapsed * 12.0) % 3;
  else if (game.game_over) frame = 2;
  if (auto res = draw_bitmap(origin + float2{bird_x, game.bird_y}, float2::fill(bird_size), a.birds[frame]); !res)
    res.error().go_off();
}

void draw_game(ui::blank& canvas, const game_state& game, const assets& a) {
  const auto origin = canvas.pos();
  const auto size = canvas.size();

  fill_rect(origin, {}, size, color(0.43f, 0.78f, 0.92f));
  fill_rect(origin, {0.0f, board_height * 0.62f}, {board_width, board_height * 0.25f}, color(0.55f, 0.84f, 0.72f));
  draw_cloud(origin, {102.0f, 124.0f}, 1.0f);
  draw_cloud(origin, {352.0f, 86.0f}, 0.78f);
  draw_cloud(origin, {428.0f, 216.0f}, 1.12f);

  for (const auto& p : game.pipes) draw_pipe(origin, p);
  draw_ground(origin, game);
  draw_bird(origin, game, a);

  if (!game.running) {
    text prompt(game.game_over ? L"Game over - flap to retry" : L"Space / Up / W", {.size = 27.0f});
    prompt.color(color(0.98f, 0.98f, 0.96f));
    const auto p = origin + float2{(board_width - prompt.size().x) * 0.5f, board_height * 0.36f};
    if (auto res = draw_text(p + float2{2.0f, 2.0f}, prompt.color(color(0.20f, 0.23f, 0.22f, 0.55f))); !res)
      res.error().go_off();
    if (auto res = draw_text(p, prompt.color(color(0.98f, 0.98f, 0.96f))); !res) res.error().go_off();
  }
}

} // namespace

int main() {
  auto win = window({.title = L"flappy bird", .resizable = false});
  win.background_color(color(0.17f, 0.20f, 0.21f));

  const auto a = load_assets();

  auto root = ui::grid_layout(win);
  if (auto res = root.columns(ui::free, ui::free); !res) res.error().go_off();
  if (auto res = root.rows(40.0f, board_height); !res) res.error().go_off();
  root.gap({8.0f, 10.0f}).padding(float4::fill(12.0f));

  auto score = ui::label(root.at({0, 0}));
  auto best = ui::label(root.at({1, 0}));
  auto canvas = ui::blank(root.at({0, 1}, {2, 1}));
  canvas.size({board_width, board_height});

  score.text_align(ui::left).background_color(colors::transparent).text_color(color(0.94f, 0.96f, 0.96f));
  best.text_align(ui::right).background_color(colors::transparent).text_color(color(0.94f, 0.96f, 0.96f));

  game_state game;
  game.restart();
  int best_score = 0;

  win.key_event([&](key_event e) {
    if (!e.down) return false;
    if (e.key != keys::space && e.key != keys::up && e.key != keys::w) return false;
    flap(game);
    return true;
  });

  while (mainloop) {
    const auto dt = float(yw::min(mainloop.spf(), 1.0 / 30.0));

    update_game(game, a, dt);
    best_score = yw::max(best_score, game.score);
    score.string(format<wchar_t>(L"Score ", game.score));
    best.string(format<wchar_t>(L"Best ", best_score));

    if (auto d = win.begin_draw()) {
      draw_game(canvas, game, a);
    } else d.error().go_off();
  }
}
