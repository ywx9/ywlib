// #pragma once
// #include "ywx/bitmap.h"

// namespace yw {

// /// keys
// enum class key : uint8_t {
//   backspace = 0x08, tab = 0x09, enter = 0x0D, escape = 0x1B, space = 0x20,
//   page_up = 0x21, page_down = 0x22, end = 0x23, home = 0x24, left = 0x25,
//   up = 0x26, right = 0x27, down = 0x28, delete_ = 0x2E, insert = 0x2D,
//   a = 'a', b = 'b', c = 'c', d = 'd', e = 'e', f = 'f', g = 'g', h = 'h',
//   i = 'i', j = 'j', k = 'k', l = 'l', m = 'm', n = 'n', o = 'o', p = 'p', q = 'q',
//   r = 'r', s = 's', t = 't', u = 'u', v = 'v', w = 'w', x = 'x', y = 'y', z = 'z',
//   n0 = '0', n1 = '1', n2 = '2', n3 = '3', n4 = '4',
//   n5 = '5', n6 = '6', n7 = '7', n8 = '8', n9 = '9',
//   f1 = 0x70, f2 = 0x71, f3 = 0x72, f4 = 0x73, f5 = 0x74, f6 = 0x75,
//   f7 = 0x76, f8 = 0x77, f9 = 0x78, f10 = 0x79, f11 = 0x7A, f12 = 0x7B,
//   hyphen = 0xBD, semicolon = 0xBA, comma = 0xBC, period = 0xBE, slash = 0xBF
// };

// /// mouse buttons
// enum class mouse : uint8_t { unknown, left = 1, middle = 2, right = 4, extra1 = 8, extra2 = 16 };
// constexpr mouse operator|(mouse a, mouse b) noexcept { return mouse(uint8_t(a) | uint8_t(b)); }
// constexpr mouse operator&(mouse a, mouse b) noexcept { return mouse(uint8_t(a) & uint8_t(b)); }
// constexpr mouse operator^(mouse a, mouse b) noexcept { return mouse(uint8_t(a) ^ uint8_t(b)); }
// constexpr mouse operator~(mouse a) noexcept { return mouse(~uint8_t(a)); }

// /// event types
// enum class event_type : uint8_t { unknown, enter, leave, move, down, up, wheel };

// //////////////////////////////////////// MARK: event

// namespace window {

// class slot;
// class slave;
// class master;

// enum class style : DWORD {
//   unknown,
//   regular = WS_OVERLAPPEDWINDOW,
//   fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
//   borderless = WS_POPUP
// };

// struct slotid {
//   yw::slotid master;
//   yw::slotid slave;
// };

// inline struct {
//   static std::expected<master, error_trace> operator()(int2, int2, null_terminated<wchar_t>, style, bool);
//   static std::expected<master, error_trace> operator()(int2, null_terminated<wchar_t>, style, bool);
//   static std::expected<slave, error_trace> subwindow(master&, int2, int2, null_terminated<wchar_t>, style, bool);
// } open;

// //////////////////////////////////////// MARK: window::system

// inline struct {
//   friend class slot;
//   static LRESULT __stdcall proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
//   bool _initialized{};

// public:
//   HINSTANCE hinstance{};
//   std::wstring_view name{};

//   slotlist<slot> windows{};
//   error_trace last_error{};
//   uint64_t frame_count{};
//   bool escape_loop = false;
//   uint32_t max_frames_per_second = 1000;  // mutable
//   uint32_t max_messages_per_frame = 1000; // mutable

//   struct state {
//     float2 pos{}, local_pos{}, delta{};
//     uint8_t key, mouse, type, ctrl, shift, alt;
//   } state;
//   static_assert(sizeof(decltype(state)[2]) == 32 * 2);

//   std::expected<void, error_trace> initialize() {
//     if (_initialized) return {};
//     hinstance = ::GetModuleHandleW(nullptr);
//     name = L"ywlib_window_class";
//     WNDCLASSW wc{};
//     wc.lpfnWndProc = proc;
//     wc.hInstance = hinstance;
//     wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
//     wc.lpszClassName = name.data();
//     if (!::RegisterClassW(&wc)) return unexpected_win32_error("RegisterClassW failed");
//     _initialized = true;
//     return {};
//   }
// } system;

// #pragma pack(push, 4)
// struct mouse_move_event {
//   float2 pos, local_pos, delta;
//   key _0;
//   mouse button;
//   event_type type;
//   bool ctrl, shift, alt;
// };
// static_assert(sizeof(mouse_move_event) == sizeof(system.state));
// struct mouse_button_event {
//   float2 pos, local_pos, _0;
//   key _1;
//   mouse button;
//   event_type type;
//   bool ctrl, shift, alt;
// };
// static_assert(sizeof(mouse_button_event) == sizeof(system.state));
// struct mouse_wheel_event {
//   float2 pos, local_pos;
//   float delta, delta_h;
//   key _0;
//   mouse button;
//   event_type type;
//   bool ctrl, shift, alt;
// };
// static_assert(sizeof(mouse_wheel_event) == sizeof(system.state));
// struct hover_event {
//   float2 pos, local_pos, delta;
//   key _0;
//   mouse button;
//   event_type type;
//   bool ctrl, shift, alt;
// };
// static_assert(sizeof(hover_event) == sizeof(system.state));
// struct key_event {
//   float2 pos, local_pos, _0;
//   key key;
//   mouse _1;
//   event_type type;
//   bool ctrl, shift, alt;
// };
// static_assert(sizeof(key_event) == sizeof(system.state));
// #pragma pack(pop)
// } // namespace window

// namespace control {

// //////////////////////////////////////// MARK: control::slotid

// struct slotid : public window::slotid {
//   yw::slotid control;
// };

// //////////////////////////////////////// MARK: control::base

// class base {
//   base(const base&) = delete;
//   base& operator=(const base&) = delete;

// public:
//   class slot {
//     friend class ::yw::window::slot;
//     slot(const slot&) = delete;
//     slot& operator=(const slot&) = delete;

//   public:
//     control::slotid id{};
//     float2 pos{};
//     float2 size{};
//     float2 radius{};
//     color background_color = colors::white;
//     color border_color = colors::black;
//     float border_width = 1.0f;
//     bool visible = true;
//     bool enabled = true;

//     function<void, const hover_event&> on_hover;

//     virtual ~slot() noexcept = default;
//     slot(slot&&) noexcept = default;
//     slot& operator=(slot&&) noexcept = default;

//     slot(const yw::slotid& mid, const yw::slotid& sid, float2 p, float2 s) noexcept {
//       id.master = mid;
//       id.slave = sid;
//       pos = p;
//       size = s;
//     }

//     virtual bool focusable() const noexcept { return false; }
//     virtual bool hit_test(float2 pt) const noexcept {
//       return pt.x >= pos.x && pt.x <= pos.x + size.x && pt.y >= pos.y && pt.y <= pos.y + size.y;
//     }

//     virtual std::expected<void, error_trace> draw() const {
//       if (auto res = fill_round_rectangle(pos, size, radius, background_color); !res)
//         return unexpected_error(res.error().push());
//       if (auto res = draw_round_rectangle(pos, size, radius, border_color, border_width); !res)
//         return unexpected_error(res.error().push());
//       return {};
//     }

//     virtual void mouse_move(const mouse_move_event& e) {}
//     virtual void mouse_button(const mouse_button_event& e) {}
//     virtual void mouse_wheel(const mouse_wheel_event& e) {}
//     virtual void hover(const hover_event& e) {
//       if (on_hover) on_hover(e);
//     }
//     virtual void drag(const drag_event& e) {}
//     virtual void key(const key_event& e) {}
//     virtual void input(const input_event& e) {}
//   };

// protected:
//   control::slotid _id;

//   base(const yw::slotid& mid, const yw::slotid& sid, const yw::slotid& cid) noexcept {
//     _id.master = mid;
//     _id.slave = sid;
//     _id.control = cid;
//   }

//   window::slot* _window() const noexcept;
//   control::base::slot* _control() const noexcept;

//   template<typename Mp> const auto& _get(Mp mp) const {
//     if (const auto s = dynamic_cast<class_type<Mp>*>(_control())) return s->*mp;
//     else throw std::runtime_error("control is not accessible");
//   }
//   template<typename Mp, typename T> void _set(Mp mp, T&& value) const noexcept;
//   template<typename Ctrl> static std::expected<Ctrl, error_trace> add(window::slave& w, float2 p, float2 s);

// public:
//   base() noexcept = default;
//   base(base&&) noexcept = default;
//   base& operator=(base&&) noexcept = default;

//   explicit operator bool() const noexcept;

//   const auto& pos() const { return _get(&slot::pos); }
//   const auto& size() const { return _get(&slot::size); }
//   const auto& radius() const { return _get(&slot::radius); }
//   const auto& background_color() const { return _get(&slot::background_color); }
//   const auto& border_color() const { return _get(&slot::border_color); }
//   const auto& border_width() const { return _get(&slot::border_width); }
//   const auto& visible() const { return _get(&slot::visible); }
//   const auto& enabled() const { return _get(&slot::enabled); }
//   const auto& on_hover() const { return _get(&slot::on_hover); }

//   void pos(float2 p) { _set(&slot::pos, p); }
//   void size(float2 s) { _set(&slot::size, s); }
//   void radius(float2 r) { _set(&slot::radius, r); }
//   void background_color(color c) { _set(&slot::background_color, c); }
//   void border_color(color c) { _set(&slot::border_color, c); }
//   void border_width(float w) { _set(&slot::border_width, w); }
//   void visible(bool b) { _set(&slot::visible, b); }
//   void enabled(bool e) { _set(&slot::enabled, e); }
//   void on_hover(function<void, bool> f) { _set(&slot::on_hover, std::move(f)); }

//   static std::expected<base, error_trace> add(window::slave& w, float2 Pos, float2 Size) {
//     return add<base>(w, Pos, Size);
//   }
// };
// } // namespace control

// namespace window {

// //////////////////////////////////////// MARK: window::slot

// class slot {
//   friend decltype(::yw::window::open);
//   friend decltype(::yw::window::system);

//   std::expected<void, error_trace> _create_window(const wchar_t* t, window::style s) {
//     switch (this->style = s) {
//     case window::style::regular:
//     case window::style::fixed:
//     case window::style::borderless: break;
//     default: return unexpected_error(errors::invalid_argument, "invalid window style");
//     }
//     hwnd = ::CreateWindowExW(WS_EX_ACCEPTFILES, system.name.data(), t, DWORD(s), 0, 0, 0, 0, 0, 0, system.hinstance, 0);
//     if (!hwnd) return unexpected_win32_error("CreateWindowExW failed");
//     ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
//     RECT cr{}, wr{};
//     if (!::GetClientRect(hwnd, &cr)) return unexpected_win32_error("GetClientRect failed");
//     if (!::GetWindowRect(hwnd, &wr)) return unexpected_win32_error("GetWindowRect failed");
//     const auto left = (wr.right - wr.left - cr.right) / 2;
//     const auto top = wr.bottom - wr.top - cr.bottom - left;
//     margin = int4(left, top, 2 * left, left + top);
//     return {};
//   }

//   std::expected<void, error_trace> _set_possize(int2 p, int2 s) {
//     if (!::SetWindowPos(hwnd, nullptr, p.x, p.y, s.x + margin.z, s.y + margin.w, SWP_NOZORDER))
//       return unexpected_win32_error("SetWindowPos failed");
//     else return {};
//   }

//   std::expected<void, error_trace> _set_possize(int2 s) {
//     if (RECT r; !::GetClientRect(::GetDesktopWindow(), &r)) return unexpected_win32_error("GetClientRect failed");
//     else return _set_possize(int2((r.right - s.x - margin.z) / 2, (r.bottom - s.y - margin.w) / 2), s);
//   }

//   std::expected<void, error_trace> _resize_rendertarget(uint2 size) {
//     if (swapchain) {
//       rendertarget = {};
//       if (auto hr = swapchain->ResizeBuffers(0, size.x, size.y, DXGI_FORMAT_UNKNOWN, 0); FAILED(hr))
//         return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int32_t(hr));
//     } else {
//       if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
//       auto desc = DXGI_SWAP_CHAIN_DESC1(size.x, size.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
//       desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
//       auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), hwnd, &desc, nullptr, nullptr, &swapchain.get());
//       if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChainForHwnd failed", int32_t(hr));
//     }
//     if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
//     else return unexpected_error(res.error());
//     return {};
//   }

// public:
//   window::slotid id{};

//   HWND hwnd{};
//   int4 margin{};
//   window::style style{};
//   bitmap rendertarget{};
//   comptr<IDXGISwapChain1> swapchain{};
//   stopwatch timer{};

//   slotlist<window::slot> slaves{};
//   slotlist<control::base::slot> controls{};

//   yw::slotid hovered_control{};
//   yw::slotid focused_control{};

//   int2 pos{};
//   uint2 size{};
//   bool resizing = false;

//   std::wstring title;

//   color background_color = colors::white;
//   bool close_confirmation = false;

//   function<void, key_event> on_key;

//   bool visible = false;
//   bool enabled = true;
//   mutable bool dirty = true;

//   slot() noexcept = default;
//   slot(slot&&) noexcept = default;
//   slot& operator=(slot&&) noexcept = default;

//   slot(const slot&) = delete;
//   slot& operator=(const slot&) = delete;

//   control::base::slot* hit_test(float2 pt) noexcept {
//     for (auto& control_slot : controls | std::views::reverse)
//       if (control_slot.hit_test(pt)) return &control_slot;
//     return nullptr;
//   }

//   const control::base::slot* hit_test(float2 pt) const noexcept {
//     for (const auto& control_slot : controls)
//       if (control_slot.hit_test(pt)) return &control_slot;
//     return nullptr;
//   }

//   std::expected<void, error_trace> update() {
//     for (auto& slave_slot : slaves) slave_slot.update();
//     if (!dirty) return {};
//     if (auto d = rendertarget.begin_draw())
//       for (auto& control : controls)
//         if (control.visible) control.draw();
//         else return unexpected_error(d.error().push());
//     dirty = false;
//     return {};
//   }

//   void advance_focus(int32_t i) {
//     auto j = controls.find(focused_control) - controls.begin();
//     const auto n = static_cast<decltype(j)>(controls.size());
//     while (i > 0) {
//       if (j == n) j = 0, --i;
//       else if (controls[++j].focusable()) --i;
//     }
//     while (i < 0) {
//       if (j == 0) j = n, ++i;
//       else if (controls[--j].focusable()) ++i;
//     }
//   }
// };

// //////////////////////////////////////// MARK: window::slave

// class slave {
//   friend decltype(window::open);
//   friend decltype(window::system);
//   friend class ::yw::control::base;

// protected:
//   window::slotid _id{};

//   slave(const window::slotid& id, bool show) noexcept {
//     if (const auto window_slot =  system.windows.get(id.master)) {
//       _id.master = id.master;
//       _id.slave = id.slave;
//       window_slot->id.master = id.master;
//       window_slot->id.slave = id.slave;
//       window_slot->timer.start();
//       if (show) this->visible(true);
//     }
//   }

//   slot* _window() const noexcept {
//     if (const auto ms = system.windows.get(_id.master); !ms) return nullptr;
//     else return _id.slave ? ms->slaves.get(_id.slave) : ms;
//   }

//   HWND _hwnd() const noexcept {
//     const auto w = _window();
//     return w ? w->hwnd : nullptr;
//   }

//   template<typename Mp> const auto& _get(Mp mp) const {
//     if (const auto ws = _window()) return ws->*mp;
//     else throw std::logic_error("window is not accessible");
//   }

// public:
//   ~slave() noexcept { ::DestroyWindow(_hwnd()); }
//   slave() noexcept = default;
//   slave(slave&&) noexcept = default;

//   slave& operator=(slave&& w) noexcept {
//     if (this == &w) return *this;
//     ::DestroyWindow(_hwnd());
//     _id = std::move(w._id);
//     return *this;
//   }

//   explicit operator bool() const noexcept {
//     if (const auto ms = system.windows.get(_id.master); !ms) return false;
//     else return _id.slave ? ms->slaves.contains(_id.slave) : true;
//   }

//   bool visible() const noexcept { return _get(&slot::visible); }
//   bool enabled() const noexcept { return _get(&slot::enabled); }
//   const int2& pos() const { return _get(&slot::pos); }
//   const uint2& size() const { return _get(&slot::size); }
//   int2 cursor() const { return system.cursor - pos(); }
//   const std::wstring& title() const { return _get(&slot::title); }

//   void visible(bool b) {
//     if (const auto window_slot = _window()) {
//       window_slot->visible = b;
//       window_slot->dirty = b;
//       if (const auto hwnd = window_slot->hwnd; !b) ::ShowWindow(hwnd, SW_HIDE);
//       else ::ShowWindow(hwnd, SW_SHOW), ::SetForegroundWindow(hwnd), ::SetActiveWindow(hwnd);
//     }
//   }

//   void enable(bool b) {
//     if (const auto window_slot = _window())
//       if (const auto hwnd = window_slot->hwnd) {
//         window_slot->enabled = b;
//         window_slot->dirty = true;
//         ::EnableWindow(hwnd, b);
//       }
//   }

//   void pos(int2 p) {
//     ::SetWindowPos(_hwnd(), nullptr, p.x, p.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
//   }

//   void size(int2 s) {
//     if (const auto window_slot = _window()) {
//       window_slot->dirty = true;
//       ::SetWindowPos(window_slot->hwnd, nullptr, 0, 0, s.x, s.y, SWP_NOZORDER | SWP_NOMOVE);
//     }
//   }

//   void cursor(int2 c) {
//     if (const auto hwnd = _hwnd()) {
//       const auto pt = reinterpret_cast<POINT*>(&c);
//       ::ClientToScreen(hwnd, pt);
//       ::SetCursorPos(pt->x, pt->y);
//     }
//   }

//   void title(null_terminated<wchar_t> t) { ::SetWindowTextW(_hwnd(), t.data()); }

//   double time() const noexcept {
//     if (const auto w = _window()) return w->timer.elapsed();
//     else return 0.0;
//   }

//   std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
//     if (const auto window_slot = _window()) {
//       window_slot->dirty = true;
//       return window_slot->rendertarget.begin_draw(src);
//     } else return unexpected_error(errors::not_initialized, "window not initialized");
//   }

//   std::expected<drawing, error_trace> begin_draw(color clear_color, const source& src = {}) {
//     if (const auto window_slot = _window()) {
//       window_slot->dirty = true;
//       return window_slot->rendertarget.begin_draw(clear_color, src);
//     } else return unexpected_error(errors::not_initialized, "window not initialized");
//   }

//   void screenshot(const std::filesystem::path& Png) const noexcept {
//     if (const auto window_slot = _window()) window_slot->rendertarget.save_as_png(Png);
//   }

//   void background_color(const color& c) noexcept {
//     if (const auto window_slot = _window()) window_slot->background_color = c;
//   }

//   void close_confirmation(bool b = true) noexcept {
//     if (const auto window_slot = _window()) window_slot->close_confirmation = b;
//   }
// };

// //////////////////////////////////////// MARK: window::master

// class master : public slave {
//   friend decltype(window::open);
//   friend decltype(window::system);

// public:
//   using slave::slave;

//   std::expected<slave, error_trace> open_subwindow(
//     int2 Offset, int2 Size, const null_terminated<wchar_t>& Title, style Style = style::unknown, bool Show = true) {
//     if (auto res = open.subwindow(*this, Offset, Size, Title, Style, Show)) return std::move(*res);
//     else return unexpected_error(res.error());
//   }
// };

// namespace internal {

// //////////////////////////////////////// MARK: window::internal::mousemove

// inline std::expected<int, error_trace> mousemove(window::slot& ws, WPARAM wp, LPARAM lp) {
//   const int2 new_pos = {static_cast<int16_t>(lp & 0xFFFF), static_cast<int16_t>((lp >> 16) & 0xFFFF)};
//   const int2 delta = new_pos - std::exchange(window::system.cursor, new_pos);
//   const int2 local_pos = new_pos - ws.pos;
//   const bool ctrl = bool(wp & MK_CONTROL);
//   const bool shift = bool(wp & MK_SHIFT);
//   const bool alt = bool(GetAsyncKeyState(VK_MENU) & 0x8000);
//   // mouse_move_event
//   if (ws.focused_control)
//     if (const auto cs = ws.controls.get(ws.focused_control))
//       cs->mouse_move({new_pos, local_pos, delta, ctrl, shift, alt});
//   // check hovered control
//   if (const auto hit = ws.hit_test(local_pos); !hit) {
//     if (ws.hovered_control)
//       if (auto hovered_cs = ws.controls.get(ws.hovered_control))
//         if (hovered_cs->on_hover) hovered_cs->on_hover({new_pos, local_pos, delta, hover_event::type::leave, ctrl, shift, alt});
//   }

//   if (hit) {
//     if (hit->id.control == ws.hovered_control) hit->hover({new_pos, local_pos, delta, hover_event::type::move, ctrl, shift, alt});
//     else {
//       if (ws.hovered_control)
//         if (auto hovered_cs = ws.controls.get(ws.hovered_control))
//           if (hovered_cs->on_hover) hovered_cs->on_hover({new_pos, local_pos, delta, hover_event::type::leave, ctrl, shift, alt});
//       ws.hovered_control = hit->id.control;
//       hit->hover({new_pos, local_pos, delta, hover_event::type::enter, ctrl, shift, alt});
//     }
//   }
//   // コントロールが見つからなければ、hovered_controlをクリア。
//   if (ws.hovered_control) {
//     if (auto hovered_cs = ws.controls.get(ws.hovered_control))
//       if (hovered_cs->on_hover) hovered_cs->on_hover(false);
//     ws.hovered_control = {};
//   }
//   return 0;
// }
// }

// } // namespace window

// //////////////////////////////////////// MARK: mainloop

// inline class {
// public:
//   enum class state { running, quit, error } state = {};
//   bool skip_drawing = false;
//   stopwatch timer;

//   bool operator()() {
//     if (!skip_drawing) {
//       timer.start();
//       for (auto& master_slot : window::system.windows) {
//         if (auto d = master_slot.rendertarget.begin_draw())
//           for (auto& control : master_slot.controls)
//             if (control.visible) control.draw();
//         if (master_slot.swapchain) master_slot.swapchain->Present(0, 0);
//         for (auto& slave_slot : master_slot.slaves) {
//           if (auto d = slave_slot.rendertarget.begin_draw())
//             for (auto& control : slave_slot.controls)
//               if (control.visible) control.draw();
//           if (slave_slot.swapchain) slave_slot.swapchain->Present(0, 0);
//         }
//       }
//     }
//     ++window::system.frame_count;
//     uint32_t processed_messages = 0;
//     for (MSG msg; ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);) {
//       if (msg.message == WM_QUIT) return state = state::quit, false;
//       ::TranslateMessage(&msg);
//       ::DispatchMessageW(&msg);
//       if (window::system.last_error) return state = state::error, false;
//       if (++processed_messages >= window::system.max_messages_per_frame) break;
//     }
//     if (skip_drawing = timer.elapsed() <= 1.0 / window::system.max_frames_per_second; !skip_drawing) timer.restart();
//     return state == state::running;
//   }

//   /// `while (mainloop)`
//   explicit operator bool() { return this->operator()(); }
// } mainloop;

// //////////////////////////////////////// MARK: window::system.proc

// inline LRESULT __stdcall decltype(window::system)::proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
//   auto ws = reinterpret_cast<window::slot*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
//   if (!ws) return ::DefWindowProcW(hwnd, msg, wp, lp);
//   switch (msg) {
//   case WM_MOUSEMOVE:
//     if (auto res = window::internal::mousemove(*ws, wp, lp); !res) {
//       window::system.last_error = std::move(res.error().push());
//       return 0;
//     } else return res.value();
//   }
// }

// //////////////////////////////////////// MARK: implementation

// inline std::expected<window::master, error_trace> decltype(window::open)::operator()(
//   int2 Pos, int2 Size, null_terminated<wchar_t> Title, window::style Style = window::style::regular, bool show = true) {
//   if (auto res = window::system.initialize(); !res) return unexpected_error(res.error());
//   switch (Style) {
//   case window::style::regular:
//   case window::style::fixed:
//   case window::style::borderless: break;
//   default: return unexpected_error(errors::invalid_argument, "invalid window style");
//   }
//   auto ws = std::make_unique<window::slot>();
//   if (auto res = ws->_create_window(Title.data(), Style); !res) return unexpected_error(res.error());
//   if (auto res = ws->_set_possize(Pos, Size); !res) return unexpected_error(res.error());
//   const auto id = window::system.windows.push(std::move(ws));
//   return window::master({id, {}}, show);
// }

// inline std::expected<window::master, error_trace> decltype(window::open)::operator()(
//   int2 Size, null_terminated<wchar_t> Title, window::style Style = window::style::regular, bool show = true) {
//   if (auto res = window::system.initialize(); !res) return unexpected_error(res.error());
//   switch (Style) {
//   case window::style::regular:
//   case window::style::fixed:
//   case window::style::borderless: break;
//   default: return unexpected_error(errors::invalid_argument, "invalid window style");
//   }
//   auto ws = std::make_unique<window::slot>();
//   if (auto res = ws->_create_window(Title.data(), Style); !res) return unexpected_error(res.error());
//   if (auto res = ws->_set_possize(Size); !res) return unexpected_error(res.error());
//   const auto id = window::system.windows.push(std::move(ws));
//   return window::master({id, {}}, show);
// }

// inline std::expected<window::slave, error_trace> decltype(window::open)::subwindow(
//   master& Window, int2 Pos, int2 Size, null_terminated<wchar_t> Title, style Style = style::unknown, bool Show = true) {
//   const auto mid = Window._id.master;
//   const auto ms = window::system.windows.get(mid);
//   if (!ms) return unexpected_error(errors::invalid_argument, "invalid master window");
//   switch (Style) {
//   case window::style::unknown: Style = ms->style; break;
//   case window::style::regular:
//   case window::style::fixed:
//   case window::style::borderless: break;
//   default: return unexpected_error(errors::invalid_argument, "invalid window style");
//   }
//   auto ss = std::make_unique<window::slot>();
//   if (auto res = ss->_create_window(Title.data(), Style); !res) return unexpected_error(res.error());
//   if (auto res = ss->_set_possize(ms->pos + Pos, Size); !res) return unexpected_error(res.error());
//   const auto sid = ms->slaves.push(std::move(ss));
//   return window::slave({mid, sid}, Show);
// }


// inline window::slot* control::base::_window() const noexcept {
//   if (const auto ms = window::system.windows.get(_id.master); !ms) return nullptr;
//   else return _id.slave ? ms->slaves.get(_id.slave) : ms;
// }

// inline control::base::slot* control::base::_control() const noexcept {
//   if (const auto ws = _window(); !ws) return nullptr;
//   else return ws->controls.get(_id.control);
// }
// } // namespace yw
