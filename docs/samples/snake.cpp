#include <ywxlib>

using namespace yw;

namespace {

constexpr int2 board_cells = {28, 20};
constexpr float cell = 24.0f;
constexpr float2 board_size = float2(board_cells) * cell;
constexpr double step_seconds = 0.15;

struct snake_part {
  int2 pos;
};

struct game_state {
  std::vector<snake_part> snake;
  int2 direction = {1, 0};
  int2 next_direction = {1, 0};
  int2 food = {18, 10};
  double accumulator = 0.0;
  double elapsed = 0.0;
  int score = 0;
  bool running = false;
  bool game_over = false;

  bool occupies(int2 p) const {
    return std::ranges::any_of(snake, [p](const snake_part& part) { return part.pos == p; });
  }

  void place_food() {
    for (int i = 0; i < board_cells.x * board_cells.y; ++i) {
      const int2 p = {yw::random::gen<int>(0, board_cells.x - 1), yw::random::gen<int>(0, board_cells.y - 1)};
      if (!occupies(p)) {
        food = p;
        return;
      }
    }
    for (int y = 0; y < board_cells.y; ++y)
      for (int x = 0; x < board_cells.x; ++x)
        if (const int2 p = {x, y}; !occupies(p)) {
          food = p;
          return;
        }
  }

  void restart() {
    snake.clear();
    const int2 head = board_cells / 2;
    snake.push_back({head});
    snake.push_back({head - int2{1, 0}});
    snake.push_back({head - int2{2, 0}});
    direction = {1, 0};
    next_direction = direction;
    accumulator = 0.0;
    elapsed = 0.0;
    score = 0;
    running = false;
    game_over = false;
    place_food();
  }
};

bool opposite(int2 a, int2 b) { return a.x + b.x == 0 && a.y + b.y == 0; }

void turn(game_state& game, int2 dir) {
  if (!opposite(game.direction, dir)) game.next_direction = dir;
}

struct input_state {
  bool launch = false;

  bool set(game_state& game, key k, bool down) {
    if (!down) {
      if (k == keys::space || k == keys::up || k == keys::w || k == keys::down || k == keys::s ||
          k == keys::left || k == keys::a || k == keys::right || k == keys::d)
        return true;
      return false;
    }

    if (k == keys::up || k == keys::w) return turn(game, {0, -1}), true;
    if (k == keys::down || k == keys::s) return turn(game, {0, 1}), true;
    if (k == keys::left || k == keys::a) return turn(game, {-1, 0}), true;
    if (k == keys::right || k == keys::d) return turn(game, {1, 0}), true;
    if (k == keys::space) {
      launch = true;
      return true;
    }
    return false;
  }
};

void draw_cell(float2 origin, int2 p, const color& c, float inset = 2.0f) {
  brush::color(c);
  fill_rectangle(origin + float2(p) * cell + float2::fill(inset), float2::fill(cell - inset * 2.0f));
}

void draw_game(ui::blank& canvas, const game_state& game) {
  const auto origin = canvas.pos();
  const auto size = canvas.size();

  brush::color(color(0.04f, 0.07f, 0.06f));
  if (auto res = fill_rectangle(origin, size); !res) res.error().go_off();

  brush::color(color(0.46f, 0.58f, 0.54f));
  if (auto res = stroke_rectangle(origin, size); !res) res.error().go_off();

  brush::color(color(0.09f, 0.14f, 0.12f));
  for (int y = 1; y < board_cells.y; ++y)
    if (auto res = stroke_line(origin + float2{0.0f, float(y) * cell}, origin + float2{board_size.x, float(y) * cell});
      !res)
      res.error().go_off();
  for (int x = 1; x < board_cells.x; ++x)
    if (auto res = stroke_line(origin + float2{float(x) * cell, 0.0f}, origin + float2{float(x) * cell, board_size.y});
      !res)
      res.error().go_off();

  draw_cell(origin, game.food, color(0.95f, 0.23f, 0.22f), 4.0f);

  for (size_t i = game.snake.size(); i > 0; --i) {
    const auto index = i - 1;
    const auto shade = 0.42f + float(index % 4) * 0.055f;
    draw_cell(origin, game.snake[index].pos, color(0.18f, shade, 0.35f), index == 0 ? 1.5f : 2.5f);
  }

  if (game.snake.size()) draw_cell(origin, game.snake.front().pos, color(0.55f, 0.92f, 0.62f), 1.5f);
}

void advance(game_state& game) {
  game.direction = game.next_direction;
  const auto next = game.snake.front().pos + game.direction;

  if (next.x < 0 || next.x >= board_cells.x || next.y < 0 || next.y >= board_cells.y) {
    game.running = false;
    game.game_over = true;
    return;
  }

  const bool eating = next == game.food;
  const auto tail = game.snake.back().pos;
  if (std::ranges::any_of(
        game.snake, [&](const snake_part& part) { return part.pos == next && (eating || part.pos != tail); })) {
    game.running = false;
    game.game_over = true;
    return;
  }

  game.snake.insert(game.snake.begin(), {next});
  if (eating) {
    game.score += 10;
    game.place_food();
  } else game.snake.pop_back();
}

void update_game(game_state& game, input_state& input, float dt) {
  if (input.launch) {
    if (game.game_over) game.restart();
    game.running = true;
    input.launch = false;
  }
  if (!game.running) return;

  game.elapsed += dt;
  game.accumulator += dt;
  while (game.accumulator >= step_seconds) {
    game.accumulator -= step_seconds;
    advance(game);
    if (!game.running) break;
  }
}
} // namespace

int main() {
  auto win = window({.title = L"snake", .resizable = false});

  auto root = ui::grid_layout(win);
  if (auto res = root.columns(board_size.x / 3.0f, board_size.x / 3.0f, board_size.x / 3.0f); !res)
    res.error().go_off();
  if (auto res = root.rows(ui::fit, ui::fit); !res) res.error().go_off();

  auto score = ui::label(root.at({0, 0}));
  auto length = ui::label(root.at({1, 0}));
  auto time = ui::label(root.at({2, 0}));
  auto board = ui::layer(root.at({0, 1}, {3, 1}));
  auto canvas = ui::blank(board);
  auto prompt = ui::label(board);

  score.text_align(ui::left);
  time.text_align(ui::right);
  canvas.size(board_size);
  prompt.text_align(ui::center).font({.size = 26.0f});

  game_state game;
  input_state input;
  game.restart();

  win.key_event([&](key_event e) {
    return input.set(game, e.key, e.down);
  });

  while (mainloop) {
    const auto dt = float(yw::min(mainloop.spf(), 0.25));

    update_game(game, input, dt);
    score.string(format<wchar_t>(L"Score ", game.score));
    length.string(format<wchar_t>(L"Length ", int(game.snake.size())));
    time.string(format<wchar_t>(L"Time ", int(game.elapsed)));
    prompt.string(game.game_over ? L"Game over - Space: restart" : L"Space: start").visible(!game.running);

    if (auto d = win.begin_draw()) {
      draw_game(canvas, game);
    } else d.error().go_off();
  }
}
