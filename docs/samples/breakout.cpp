#include <ywxlib>

using namespace yw;

namespace {

constexpr float board_width = 720.0f;
constexpr float board_height = 520.0f;
constexpr float paddle_width = 96.0f;
constexpr float paddle_height = 14.0f;
constexpr float ball_radius = 7.0f;
constexpr int brick_columns = 10;
constexpr int brick_rows = 6;

struct brick {
  float2 pos;
  float2 size;
  bool alive = true;
};

struct game_state {
  std::vector<brick> bricks;
  float paddle_x = (board_width - paddle_width) * 0.5f;
  float2 ball_pos = {board_width * 0.5f, board_height - 76.0f};
  float2 ball_vel = {210.0f, -250.0f};
  double last_time = 0.0;
  double elapsed = 0.0;
  int score = 0;
  int lives = 3;
  bool running = false;

  void reset_bricks() {
    bricks.clear();
    const float gap = 8.0f;
    const float top = 46.0f;
    const float left = 24.0f;
    const float width = (board_width - left * 2.0f - gap * float(brick_columns - 1)) / float(brick_columns);
    for (int row = 0; row < brick_rows; ++row) {
      for (int column = 0; column < brick_columns; ++column) {
        bricks.push_back({
          .pos = {left + float(column) * (width + gap), top + float(row) * 24.0f},
          .size = {width, 16.0f},
          .alive = true,
        });
      }
    }
  }

  void reset_ball() {
    ball_pos = {paddle_x + paddle_width * 0.5f, board_height - 76.0f};
    ball_vel = {210.0f, -250.0f};
    running = false;
  }

  void restart() {
    score = 0;
    lives = 3;
    elapsed = 0.0;
    paddle_x = (board_width - paddle_width) * 0.5f;
    reset_bricks();
    reset_ball();
  }
};

bool overlaps(float2 a_pos, float2 a_size, float2 b_pos, float2 b_size) {
  return a_pos.x < b_pos.x + b_size.x && b_pos.x < a_pos.x + a_size.x &&
         a_pos.y < b_pos.y + b_size.y && b_pos.y < a_pos.y + a_size.y;
}

void fill_rect(float2 origin, float2 pos, float2 size, const color& c) {
  brush::color(c);
  if (auto res = fill_rectangle(origin + pos, size); !res) res.error().go_off();
}

void stroke_rect(float2 origin, float2 pos, float2 size, const color& c, float width = 1.0f) {
  brush::color(c);
  if (auto res = stroke_rectangle(origin + pos, size, width); !res) res.error().go_off();
}

void draw_game(ui::blank& canvas, const game_state& game) {
  const auto origin = canvas.pos();
  const auto size = canvas.size();

  fill_rect(origin, {}, size, color(0.05f, 0.06f, 0.08f));
  stroke_rect(origin, {}, size, color(0.62f, 0.68f, 0.72f), 1.0f);

  for (size_t i = 0; i < game.bricks.size(); ++i) {
    const auto& b = game.bricks[i];
    if (!b.alive) continue;
    const auto row = int(i) / brick_columns;
    const color palette[] = {
      color(0.94f, 0.27f, 0.34f),
      color(0.98f, 0.56f, 0.22f),
      color(0.96f, 0.78f, 0.26f),
      color(0.20f, 0.72f, 0.48f),
      color(0.18f, 0.60f, 0.86f),
      color(0.58f, 0.48f, 0.86f),
    };
    fill_rect(origin, b.pos, b.size, palette[row % brick_rows]);
  }

  fill_rect(origin, {game.paddle_x, board_height - 34.0f}, {paddle_width, paddle_height}, color(0.86f, 0.90f, 0.94f));

  brush::color(color(0.94f, 0.97f, 1.0f));
  if (auto res = fill_ellipse(origin + game.ball_pos, float2::fill(ball_radius)); !res) res.error().go_off();

  if (!game.running) {
    text prompt(game.lives > 0 ? L"Space: start / restart" : L"Space: new game", {.size = 24.0f});
    prompt.color(color(0.90f, 0.94f, 0.98f));
    const auto p = origin + (size - prompt.size()) * 0.5f;
    if (auto res = draw_text(p, prompt); !res) res.error().go_off();
  }
}

void update_game(game_state& game, float dt) {
  if (keys::left.pressed() || keys::a.pressed()) game.paddle_x -= 460.0f * dt;
  if (keys::right.pressed() || keys::d.pressed()) game.paddle_x += 460.0f * dt;
  game.paddle_x = clamp(game.paddle_x, 0.0f, board_width - paddle_width);

  if (keys::space.pressed()) {
    if (game.lives <= 0 || std::ranges::none_of(game.bricks, [](const brick& b) { return b.alive; })) game.restart();
    game.running = true;
  }

  if (!game.running) {
    game.ball_pos.x = game.paddle_x + paddle_width * 0.5f;
    return;
  }

  game.elapsed += dt;
  game.ball_pos += game.ball_vel * dt;

  if (game.ball_pos.x - ball_radius < 0.0f) {
    game.ball_pos.x = ball_radius;
    game.ball_vel.x = yw::abs(game.ball_vel.x);
  } else if (game.ball_pos.x + ball_radius > board_width) {
    game.ball_pos.x = board_width - ball_radius;
    game.ball_vel.x = -yw::abs(game.ball_vel.x);
  }

  if (game.ball_pos.y - ball_radius < 0.0f) {
    game.ball_pos.y = ball_radius;
    game.ball_vel.y = yw::abs(game.ball_vel.y);
  }

  const float2 ball_box = float2::fill(ball_radius * 2.0f);
  const float2 ball_min = game.ball_pos - float2::fill(ball_radius);
  const float2 paddle_pos = {game.paddle_x, board_height - 34.0f};
  const float2 paddle_size = {paddle_width, paddle_height};
  if (game.ball_vel.y > 0.0f && overlaps(ball_min, ball_box, paddle_pos, paddle_size)) {
    const auto hit = ((game.ball_pos.x - game.paddle_x) / paddle_width - 0.5f) * 2.0f;
    game.ball_pos.y = paddle_pos.y - ball_radius;
    game.ball_vel.x = hit * 260.0f;
    game.ball_vel.y = -yw::abs(game.ball_vel.y);
  }

  for (auto& b : game.bricks) {
    if (!b.alive || !overlaps(ball_min, ball_box, b.pos, b.size)) continue;
    b.alive = false;
    game.score += 10;
    const auto ball_center = game.ball_pos;
    const auto brick_center = b.pos + b.size * 0.5f;
    const auto delta = ball_center - brick_center;
    if (yw::abs(delta.x / b.size.x) > yw::abs(delta.y / b.size.y)) game.ball_vel.x = -game.ball_vel.x;
    else game.ball_vel.y = -game.ball_vel.y;
    break;
  }

  if (game.ball_pos.y - ball_radius > board_height) {
    --game.lives;
    game.reset_ball();
  }

  if (std::ranges::none_of(game.bricks, [](const brick& b) { return b.alive; })) game.running = false;
}

} // namespace

int main() {
  auto win = window({.title = L"breakout", .resizable = false});
  win.background_color(color(0.12f, 0.13f, 0.15f));

  auto root = ui::grid_layout(win);
  if (auto res = root.columns(ui::free, ui::free, ui::free); !res) res.error().go_off();
  if (auto res = root.rows(38.0f, board_height); !res) res.error().go_off();
  root.gap({8.0f, 10.0f}).padding(float4::fill(12.0f));

  auto score = ui::label(root.at({0, 0}));
  auto time = ui::label(root.at({1, 0}));
  auto lives = ui::label(root.at({2, 0}));
  auto canvas = ui::blank(root.at({0, 1}, {3, 1}));
  canvas.size({board_width, board_height});

  score.text_align(ui::left).background_color(colors::transparent).text_color(color(0.92f, 0.94f, 0.96f));
  time.background_color(colors::transparent).text_color(color(0.92f, 0.94f, 0.96f));
  lives.text_align(ui::right).background_color(colors::transparent).text_color(color(0.92f, 0.94f, 0.96f));

  game_state game;
  game.restart();

  while (mainloop) {
    const auto now = mainloop.elapsed();
    const auto dt = float(yw::min(now - game.last_time, 1.0 / 30.0));
    game.last_time = now;

    update_game(game, dt);
    score.string(format<wchar_t>(L"Score ", game.score));
    time.string(format<wchar_t>(L"Time ", int(game.elapsed)));
    lives.string(format<wchar_t>(L"Lives ", yw::max(game.lives, 0)));

    if (auto d = win.begin_draw()) {
      draw_game(canvas, game);
    } else d.error().go_off();
  }
}
