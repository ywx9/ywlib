#include <ywxlib>

using namespace yw;

namespace {

constexpr int view_width = 960;
constexpr int view_height = 600;
constexpr int max_cubes = 64;
constexpr int start_lives = 5;
constexpr int grid_extent = 30;
constexpr int grid_z_start = 4;
constexpr int grid_z_end = 84;
constexpr int grid_step = 4;

constexpr const char* cube_hlsl = R"(
struct Camera {
  row_major float4x4 view;
  row_major float4x4 projection;
  row_major float4x4 view_projection;
  row_major float4x4 inverse_view;
  row_major float4x4 inverse_projection;
  row_major float4x4 inverse_view_projection;
  float4 camera_pos;
  float4 camera_dir;
  float4 viewport_size;
};

struct Cube {
  float4 pos_size;
  float4 color;
};

struct LineVertex {
  float4 pos;
  float4 color;
};

cbuffer camera_buffer : register(b0) {
  Camera camera;
};

StructuredBuffer<Cube> cubes : register(t0);
StructuredBuffer<LineVertex> line_vertices : register(t0);

struct vs_out {
  float4 pos : SV_POSITION;
  float4 color : COLOR0;
};

float3 cube_vertex(uint i) {
  switch (i) {
  case 0: return float3(-1.0, -1.0, -1.0);
  case 1: return float3(-1.0,  1.0, -1.0);
  case 2: return float3( 1.0,  1.0, -1.0);
  case 3: return float3( 1.0, -1.0, -1.0);
  case 4: return float3(-1.0, -1.0,  1.0);
  case 5: return float3(-1.0,  1.0,  1.0);
  case 6: return float3( 1.0,  1.0,  1.0);
  default: return float3( 1.0, -1.0,  1.0);
  }
}

uint cube_index(uint i) {
  static const uint indices[36] = {
    4, 5, 6, 4, 6, 7,
    3, 2, 1, 3, 1, 0,
    0, 1, 5, 0, 5, 4,
    7, 6, 2, 7, 2, 3,
    1, 2, 6, 1, 6, 5,
    0, 4, 7, 0, 7, 3
  };
  return indices[i];
}

float face_light(uint i) {
  switch (i / 6) {
  case 0: return 1.00;
  case 1: return 0.48;
  case 2: return 0.68;
  case 3: return 0.76;
  case 4: return 0.92;
  default: return 0.56;
  }
}

vs_out vsmain(uint vertex_id : SV_VertexID) {
  const uint cube_id = vertex_id / 36;
  const uint local_id = vertex_id % 36;
  const Cube cube = cubes[cube_id];
  const float3 local = cube_vertex(cube_index(local_id));
  const float3 world = cube.pos_size.xyz + local * cube.pos_size.w;

  vs_out o;
  o.pos = mul(camera.view_projection, float4(world, 1.0));
  o.color = float4(cube.color.rgb * face_light(local_id), cube.color.a);
  return o;
}

float4 psmain(vs_out input) : SV_TARGET {
  return input.color;
}

vs_out grid_vsmain(uint vertex_id : SV_VertexID) {
  const LineVertex v = line_vertices[vertex_id];

  vs_out o;
  o.pos = mul(camera.view_projection, float4(v.pos.xyz, 1.0));
  o.color = v.color;
  return o;
}
)";

struct gpu_cube {
  float4 pos_size;
  color color;
};

struct gpu_line_vertex {
  float4 pos;
  color color;
};

struct target_cube {
  float3 pos;
  float speed = 10.0f;
  float size = 1.0f;
  float3 drift;
  color col;
};

struct shot_flash {
  float life = 0.0f;
  bool hit = false;
};

struct game_state {
  std::vector<target_cube> cubes;
  std::array<gpu_cube, max_cubes> gpu_cubes{};
  double elapsed = 0.0;
  float spawn_timer = 0.0f;
  float fire_cooldown = 0.0f;
  float yaw = 0.0f;
  float pitch = 0.0f;
  int score = 0;
  int lives = start_lives;
  int wave = 1;
  bool running = false;
  bool game_over = false;
  shot_flash flash;

  void restart() {
    cubes.clear();
    elapsed = 0.0;
    spawn_timer = 0.35f;
    fire_cooldown = 0.0f;
    yaw = 0.0f;
    pitch = 0.0f;
    score = 0;
    lives = start_lives;
    wave = 1;
    running = true;
    game_over = false;
    flash = {};
  }
};

target_cube make_cube(int wave) {
  const float z = random::gen<float>(52.0f, 76.0f);
  const float spread = 11.0f + float(wave) * 0.55f;
  const color palette[] = {
    color(0.20f, 0.72f, 0.92f),
    color(0.95f, 0.46f, 0.28f),
    color(0.45f, 0.86f, 0.42f),
    color(0.94f, 0.82f, 0.26f),
    color(0.74f, 0.52f, 0.92f),
  };
  return {
    .pos = {random::gen<float>(-spread, spread), random::gen<float>(-7.0f, 8.5f), z},
    .speed = random::gen<float>(8.0f + float(wave) * 0.7f, 14.0f + float(wave) * 1.0f),
    .size = random::gen<float>(0.85f, 1.65f),
    .drift = {random::gen<float>(-1.2f, 1.2f), random::gen<float>(-0.45f, 0.45f), 0.0f},
    .col = palette[random::gen<int>(0, int(std::size(palette) - 1))],
  };
}

void spawn_cube(game_state& game) {
  if (int(game.cubes.size()) >= max_cubes) return;
  game.cubes.push_back(make_cube(game.wave));
}

void lose_life(game_state& game) {
  --game.lives;
  game.flash = {.life = 0.18f, .hit = false};
  if (game.lives <= 0) {
    game.running = false;
    game.game_over = true;
  }
}

bool fire(game_state& game, float3 dir) {
  if (!game.running || game.fire_cooldown > 0.0f) return false;
  game.fire_cooldown = 0.14f;

  constexpr float hit_radius_scale = 0.92f;
  int hit_index = -1;
  float best_t = inf;

  for (int i = 0; i < int(game.cubes.size()); ++i) {
    const auto& c = game.cubes[size_t(i)];
    const auto t = dot(c.pos, dir);
    if (t <= 0.0f || t >= best_t) continue;
    const auto closest = dir * t;
    const auto d2 = (c.pos - closest).squared_length();
    const auto radius = c.size * hit_radius_scale;
    if (d2 <= radius * radius) {
      best_t = t;
      hit_index = i;
    }
  }

  if (hit_index < 0) {
    game.flash = {.life = 0.08f, .hit = false};
    return false;
  }

  game.score += 100 + yw::max(0, 40 - int(best_t));
  game.cubes.erase(game.cubes.begin() + hit_index);
  game.flash = {.life = 0.12f, .hit = true};
  if (game.score / 900 + 1 > game.wave) ++game.wave;
  return true;
}

void update_game(game_state& game, bool fire_requested, float dt, float3 aim_dir) {
  if (!game.running) {
    if (fire_requested) game.restart();
    return;
  }

  game.elapsed += dt;
  game.spawn_timer -= dt;
  game.fire_cooldown = yw::max(0.0f, game.fire_cooldown - dt);
  game.flash.life = yw::max(0.0f, game.flash.life - dt);

  const float spawn_interval = yw::max(0.28f, 1.05f - float(game.wave) * 0.055f);
  while (game.spawn_timer <= 0.0f) {
    spawn_cube(game);
    game.spawn_timer += spawn_interval;
  }

  for (auto& c : game.cubes) {
    c.pos += c.drift * dt;
    c.pos.z -= c.speed * dt;
  }

  for (size_t i = 0; i < game.cubes.size();) {
    if (game.cubes[i].pos.z < 1.6f) {
      game.cubes.erase(game.cubes.begin() + ptrdiff_t(i));
      lose_life(game);
    } else ++i;
  }

  if (fire_requested) fire(game, aim_dir);
}

size_t update_gpu_cubes(game_state& game) {
  const auto count = yw::min(game.cubes.size(), game.gpu_cubes.size());
  for (size_t i = 0; i < count; ++i) {
    const auto& c = game.cubes[i];
    game.gpu_cubes[i].pos_size = {c.pos.x, c.pos.y, c.pos.z, c.size};
    game.gpu_cubes[i].color = c.col;
  }
  return count;
}

std::vector<gpu_line_vertex> make_grid_vertices() {
  std::vector<gpu_line_vertex> vertices;
  vertices.reserve(size_t((grid_extent / grid_step * 2 + 1) + ((grid_z_end - grid_z_start) / grid_step + 1)) * 2);

  constexpr float y = -7.5f;
  const color minor(0.34f, 0.42f, 0.50f, 0.42f);
  const color major(0.46f, 0.58f, 0.68f, 0.62f);
  const color center(0.38f, 0.78f, 0.90f, 0.82f);

  for (int x = -grid_extent; x <= grid_extent; x += grid_step) {
    const auto c = x == 0 ? center : (x % (grid_step * 2) == 0 ? major : minor);
    vertices.push_back({.pos = {float(x), y, float(grid_z_start), 0.0f}, .color = c});
    vertices.push_back({.pos = {float(x), y, float(grid_z_end), 0.0f}, .color = c});
  }

  for (int z = grid_z_start; z <= grid_z_end; z += grid_step) {
    const auto c = z % (grid_step * 2) == 0 ? major : minor;
    vertices.push_back({.pos = {-float(grid_extent), y, float(z), 0.0f}, .color = c});
    vertices.push_back({.pos = {float(grid_extent), y, float(z), 0.0f}, .color = c});
  }

  return vertices;
}

void draw_line(float2 a, float2 b, const color& c, float width = 1.0f) {
  brush::color(c);
  if (auto res = stroke_line(a, b, width); !res) res.error().go_off();
}

void fill_rect(float2 p, float2 s, const color& c) {
  brush::color(c);
  if (auto res = fill_rectangle(p, s); !res) res.error().go_off();
}

void draw_hud(const game_state& game, bool locked) {
  const auto center = float2{view_width * 0.5f, view_height * 0.5f};
  const auto reticle = game.flash.life > 0.0f && game.flash.hit ? color(0.72f, 1.0f, 0.46f) : color(0.88f, 0.96f, 1.0f);
  draw_line(center + float2{-18.0f, 0.0f}, center + float2{-6.0f, 0.0f}, reticle, 2.0f);
  draw_line(center + float2{6.0f, 0.0f}, center + float2{18.0f, 0.0f}, reticle, 2.0f);
  draw_line(center + float2{0.0f, -18.0f}, center + float2{0.0f, -6.0f}, reticle, 2.0f);
  draw_line(center + float2{0.0f, 6.0f}, center + float2{0.0f, 18.0f}, reticle, 2.0f);

  if (game.flash.life > 0.0f && !game.flash.hit) {
    fill_rect({}, {float(view_width), float(view_height)}, color(0.78f, 0.12f, 0.10f, 0.10f));
  }

  text score(format<wchar_t>(L"Score ", game.score), {.size = 22.0f});
  if (auto res = draw_text({20.0f, 16.0f}, score, color(0.92f, 0.96f, 1.0f)); !res) res.error().go_off();

  text lives(format<wchar_t>(L"Life ", yw::max(game.lives, 0)), {.size = 22.0f});
  if (auto res = draw_text({float(view_width) - lives.size().x - 20.0f, 16.0f}, lives, color(0.92f, 0.96f, 1.0f)); !res)
    res.error().go_off();

  text wave(format<wchar_t>(L"Wave ", game.wave), {.size = 18.0f});
  if (auto res = draw_text({(float(view_width) - wave.size().x) * 0.5f, 18.0f}, wave, color(0.78f, 0.86f, 0.94f)); !res)
    res.error().go_off();

  if (!game.running) {
    const auto message = game.game_over ? L"Game over - click to restart" : L"Click to start";
    text prompt(message, {.size = 30.0f});
    if (auto res =
          draw_text({(float(view_width) - prompt.size().x) * 0.5f, view_height * 0.43f}, prompt, color(0.94f, 0.98f, 1.0f));
      !res)
      res.error().go_off();
  } else if (!locked) {
    text prompt(L"Click to capture mouse", {.size = 20.0f});
    if (auto res = draw_text(
          {(float(view_width) - prompt.size().x) * 0.5f, view_height - 42.0f}, prompt,
          color(0.82f, 0.90f, 0.98f, 0.82f));
      !res)
      res.error().go_off();
  }

  text exit(L"ESC Exit", {.size = 17.0f});
  if (auto res = draw_text(
        {float(view_width) - exit.size().x - 20.0f, float(view_height) - 32.0f}, exit,
        color(0.72f, 0.80f, 0.88f, 0.78f));
    !res)
    res.error().go_off();
}

} // namespace

int main() {
  auto win = window({.title = L"3d_shooting", .size = int2(view_width, view_height), .resizable = false});
  win.background_color(color(0.015f, 0.018f, 0.026f));

  auto target = bitmap_texture(int2(view_width, view_height));
  auto depth = depth_texture(int2(view_width, view_height));
  auto cam = camera(int2(view_width, view_height));
  if (auto res = cam.perspective(1.12f); !res) res.error().go_off();
  if (auto res = cam.far_(160.0f); !res) res.error().go_off();

  auto vs = vertex_shader(cube_hlsl);
  auto grid_vs = vertex_shader(cube_hlsl, "grid_vsmain");
  auto ps = pixel_shader(cube_hlsl);
  auto cube_buffer = structured_buffer<gpu_cube>(uint1{max_cubes});
  const auto grid_vertices = make_grid_vertices();
  auto grid_buffer = structured_buffer<gpu_line_vertex>(grid_vertices);

  game_state game;
  bool fire_requested = false;

  win.pointer_event([&](pointer_event e) {
    if (!win.cursor_locked() || !game.running) return false;
    constexpr float sensitivity = 0.0022f;
    game.yaw += float(e.delta.x) * sensitivity;
    game.pitch = clamp(game.pitch + float(e.delta.y) * sensitivity, -1.15f, 1.15f);
    return true;
  });

  win.button_event([&](button_event e) {
    if (e.key != keys::lbutton || !e.down) return false;
    win.cursor_lock(true);
    fire_requested = true;
    return true;
  });

  win.key_event([&](key_event e) {
    if (!e.down) return false;
    if (e.key == keys::escape) {
      if (auto res = win.close(); !res) res.error().go_off();
      return true;
    }
    if (e.key == keys::space) {
      fire_requested = true;
      return true;
    }
    return false;
  });

  game.restart();

  while (mainloop) {
    const auto dt = float(yw::min(mainloop.spf(), 1.0 / 30.0));
    const bool shoot = std::exchange(fire_requested, false);
    if (auto res = cam.set_pose({0.0f, 0.0f, 0.0f, 0.0f}, {game.pitch, game.yaw, 0.0f, 0.0f});
        !res)
      res.error().go_off();
    update_game(game, shoot, dt, cam.forward_direction().xyz());

    const auto cube_count = update_gpu_cubes(game);
    if (auto res = cube_buffer.copy_from(game.gpu_cubes); !res) res.error().go_off();

    if (auto res = target.clear(color(0.025f, 0.032f, 0.045f)); !res) res.error().go_off();
    if (auto res = depth.clear(0.0f); !res) res.error().go_off();

    if (auto render = cam.begin_render(target, depth)) {
      if (auto res = render->set_ps(ps); !res) res.error().go_off();
      if (auto res = render->set_vs(grid_vs, cam.constant_buffer(), grid_buffer); !res) res.error().go_off();
      if (auto res = render->render_lines(uint32_t(grid_vertices.size() / 2)); !res) res.error().go_off();

      if (cube_count > 0) {
        if (auto res = render->set_vs(vs, cam.constant_buffer(), cube_buffer); !res) res.error().go_off();
        if (auto res = render->render_triangles(uint32_t(cube_count * 12)); !res) res.error().go_off();
      }
    } else render.error().go_off();

    if (auto d = win.begin_draw()) {
      if (auto res = draw_bitmap({}, target); !res) res.error().go_off();
      draw_hud(game, win.cursor_locked());
    } else d.error().go_off();
  }
}
