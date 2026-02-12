// #pragma once
// #include "ywx/text_format.h"
// #include "ywx/window_slot.h"

// namespace yw {

// class window_slot;

// ///////////////////////////////////////// MARK: control

// namespace controls {

// class control {
//   control(const control&) = delete;
//   control& operator=(const control&) = delete;

// public:
//   class slot;

// protected:
//   slotlist<window_slot>::id _master_id;
//   slotlist<window_slot>::id _slave_id;
//   slotlist<slot>::id _control_id;

//   window_slot* _window() const noexcept;
//   slot* _control() const noexcept;

//   template<typename Ctrl, typename Slot, typename W> static std::expected<Ctrl, error_trace> _add(W& Window);

// public:
//   virtual ~control() noexcept;
//   explicit control() noexcept;

//   explicit control(const slotlist<window_slot>::id& master_id, const slotlist<window_slot>::id& slave_id,
//     const slotlist<controls::control::slot>::id& control_id) noexcept;

//   control(control&&) noexcept;
//   control& operator=(control&&) noexcept;

//   explicit operator bool() const noexcept;
//   slot* operator->() const noexcept;
//   void remove() noexcept;

//   static std::expected<control, error_trace> add(auto& window);
// };
// } // namespace controls

// //////////////////////////////////////// MARK: window_slot



// inline window_slot* decltype(window_class)::get_window(HWND hwnd) noexcept {
//   if (!hwnd) return nullptr;
//   slotlist<window_slot>::id master_id;
//   constexpr int offset = sizeof(master_id.index);
//   master_id.index = ::GetWindowLongPtrW(hwnd, offset * 0);
//   master_id.generation = ::GetWindowLongPtrW(hwnd, offset * 1);
//   const auto master = window_class.windows.get(master_id);
//   if (!master) return nullptr;
//   if (master->is_slave) {
//     slotlist<window_slot>::id slave_id;
//     slave_id.index = ::GetWindowLongPtrW(hwnd, offset * 2);
//     slave_id.generation = ::GetWindowLongPtrW(hwnd, offset * 3);
//     return master->slaves.get(slave_id);
//   } else return master;
// }

// namespace window {

// class master;
// class slave;

// inline std::expected<master, error_trace> open(
//   int2 pos, int2 size, null_terminated<wchar_t> title, window_style style = window_style::regular, bool hidden = false);

// inline std::expected<master, error_trace> open(
//   int2 size, null_terminated<wchar_t> title, window_style style = window_style::regular, bool hidden = false);
// } // namespace window

// //////////////////////////////////////// MARK: window::master

// class window::master {
//   slotlist<window_slot>::id _master_id;
//   window_slot* _window() const noexcept { return window_class.windows.get(_master_id); }

// public:
//   using slot_type = window_slot;

//   ~master() noexcept { this->close(); }
//   master() noexcept : _master_id() {}
//   explicit master(slotlist<window_slot>::id mid) noexcept : _master_id(mid) {}

//   master(master&& other) noexcept : _master_id(std::exchange(other._master_id, {})) {}

//   master& operator=(master&& other) noexcept {
//     if (this == &other) return *this;
//     this->close();
//     _master_id = std::exchange(other._master_id, {});
//     return *this;
//   }

//   explicit operator bool() const noexcept { return window_class.windows.contains(_master_id); }

//   void close() noexcept {
//     if (const auto hw = hwnd(); hw) ::DestroyWindow(hw);
//   }

//   HWND hwnd() const noexcept {
//     const auto w = _window();
//     return w ? w->hwnd : HWND();
//   }

//   window_style style() const noexcept {
//     const auto w = _window();
//     return w ? w->style : window_style::unknown;
//   }

//   int2 margin() const noexcept {
//     const auto w = _window();
//     return w ? int2(w->margin.x, w->margin.y) : int2();
//   }

//   int2 position() const noexcept {
//     if (const auto hw = hwnd(); hw)
//       if (RECT r; ::GetWindowRect(hw, &r)) return int2(r.left, r.top);
//     return {};
//   }

//   int2 size() const noexcept {
//     if (const auto hw = hwnd(); hw)
//       if (RECT r; ::GetClientRect(hw, &r)) return int2(r.right - r.left, r.bottom - r.top);
//     return {};
//   }

//   int2 cursor() const noexcept {
//     const auto w = _window();
//     return w ? w->cursor_pos : int2();
//   }

//   void show(bool b) {
//     if (const auto hw = hwnd(); hw)
//       if (b) ::ShowWindow(hw, SW_SHOW), ::SetForegroundWindow(hw), ::SetActiveWindow(hw);
//       else ::ShowWindow(hw, SW_HIDE);
//   }

//   void enable(bool b) {
//     if (const auto w = _window(); w) {
//       b ? w->timer.start() : w->timer.stop();
//       if (const auto hw = w->hwnd; hw) ::EnableWindow(hw, BOOL(b));
//     }
//   }

//   void close_confirmation(bool b) {
//     if (const auto w = _window(); w) w->close_confirmation = b;
//   }

//   void position(int2 Pos) {
//     if (const auto hw = hwnd(); hw) ::SetWindowPos(hw, nullptr, Pos.x, Pos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
//   }

//   void size(int2 Size) {
//     const auto w = _window();
//     if (const auto hw = w ? w->hwnd : HWND(); hw) {
//       const auto width = Size.x + w->margin.z, height = Size.y + w->margin.w;
//       ::SetWindowPos(hw, nullptr, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
//     }
//   }

//   void cursor(int2 Pos) {
//     const auto w = _window();
//     if (const auto hw = w ? w->hwnd : HWND(); hw) {
//       POINT p{Pos.x, Pos.y};
//       ::ClientToScreen(hw, &p);
//       ::SetCursorPos(p.x, p.y);
//       window_class.cursor_pos = {p.x, p.y};
//       w->cursor_pos = Pos;
//     }
//   }

//   double elapsed_seconds() const noexcept {
//     const auto w = _window();
//     return w ? w->timer.seconds() : 0.0;
//   }

//   std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
//     if (const auto w = _window(); !w) return unexpected_error(errors::not_initialized, "window is not initialized");
//     else if (auto& rt = w->rendertarget; rt) return rt.begin_draw(src);
//     else return unexpected_error(errors::not_initialized, "window render target is not initialized");
//   }

//   std::expected<drawing, error_trace> begin_draw(const color& clear_color, const source& src = {}) {
//     if (const auto w = _window(); !w) return unexpected_error(errors::not_initialized, "window is not initialized");
//     else if (auto& rt = w->rendertarget; rt) return rt.begin_draw(clear_color, src);
//     else return unexpected_error(errors::not_initialized, "window render target is not initialized");
//   }

//   std::expected<slave, error_trace> open_subwindow(int2 offset, int2 size, null_terminated<wchar_t> title,
//     window_style style = window_style::unknown, bool hidden = false);
// };

// inline std::expected<window::master, error_trace> window::open(
//   int2 pos, int2 size, null_terminated<wchar_t> title, window_style style, bool hidden) {
//   if (auto res = window_class.initialize(); !res) return unexpected_error(res.error());
//   std::unique_ptr<window_slot> master_window = std::make_unique<window_slot>();
//   auto res = master_window->initialize(pos, size, title.data(), style, hidden, false, false);
//   if (!res) return unexpected_error(res.error());
//   const auto hwnd = master_window->hwnd;
//   const auto id = window_class.windows.push(std::move(master_window));
//   constexpr int offset = sizeof(id.index);
//   ::SetWindowLongPtrW(hwnd, offset * 0, id.index);
//   ::SetWindowLongPtrW(hwnd, offset * 1, id.generation);
//   return master(id);
// }

// inline std::expected<window::master, error_trace> window::open(
//   int2 size, null_terminated<wchar_t> title, window_style style, bool hidden) {
//   if (auto res = window_class.initialize(); !res) return unexpected_error(res.error());
//   std::unique_ptr<window_slot> master_window = std::make_unique<window_slot>();
//   auto res = master_window->initialize({}, size, title.data(), style, hidden, true, false);
//   if (!res) return unexpected_error(res.error());
//   const auto hwnd = master_window->hwnd;
//   const auto id = window_class.windows.push(std::move(master_window));
//   constexpr int offset = sizeof(id.index);
//   ::SetWindowLongPtrW(hwnd, offset * 0, id.index);
//   ::SetWindowLongPtrW(hwnd, offset * 1, id.generation);
//   return master(id);
// }

// //////////////////////////////////////// MARK: window::slave

// class window::slave {
//   slotlist<window_slot>::id _master_id;
//   slotlist<window_slot>::id _slave_id;

//   window_slot* _window() const noexcept {
//     const auto master = window_class.windows.get(_master_id);
//     return master ? master->slaves.get(_slave_id) : nullptr;
//   }

// public:
//   using slot_type = window_slot;

//   ~slave() noexcept { this->close(); }
//   slave() noexcept : _master_id(), _slave_id() {}

//   explicit slave(slotlist<window_slot>::id mid, slotlist<window_slot>::id sid) noexcept
//     : _master_id(mid), _slave_id(sid) {}

//   slave(slave&& other) noexcept
//     : _master_id(std::exchange(other._master_id, {})), _slave_id(std::exchange(other._slave_id, {})) {}

//   slave& operator=(slave&& other) noexcept {
//     if (this == &other) return *this;
//     this->close();
//     _master_id = std::exchange(other._master_id, {});
//     _slave_id = std::exchange(other._slave_id, {});
//     return *this;
//   }

//   explicit operator bool() const noexcept {
//     const auto master = window_class.windows.get(_master_id);
//     return master ? master->slaves.contains(_slave_id) : false;
//   }

//   void close() noexcept {
//     if (const auto hw = hwnd(); hw) ::DestroyWindow(hw);
//   }

//   HWND hwnd() const noexcept {
//     const auto w = _window();
//     return w ? w->hwnd : HWND();
//   }

//   window_style style() const noexcept {
//     const auto w = _window();
//     return w ? w->style : window_style::unknown;
//   }

//   int2 margin() const noexcept {
//     const auto w = _window();
//     return w ? int2(w->margin.x, w->margin.y) : int2();
//   }

//   int2 position() const noexcept {
//     if (const auto hw = hwnd(); hw)
//       if (RECT r; ::GetWindowRect(hw, &r)) return int2(r.left, r.top);
//     return {};
//   }

//   int2 size() const noexcept {
//     if (const auto hw = hwnd(); hw)
//       if (RECT r; ::GetClientRect(hw, &r)) return int2(r.right - r.left, r.bottom - r.top);
//     return {};
//   }

//   int2 cursor() const noexcept {
//     const auto w = _window();
//     return w ? w->cursor_pos : int2();
//   }

//   void show(bool b) {
//     if (const auto hw = hwnd(); hw)
//       if (b) ::ShowWindow(hw, SW_SHOW), ::SetForegroundWindow(hw), ::SetActiveWindow(hw);
//       else ::ShowWindow(hw, SW_HIDE);
//   }

//   void enable(bool b) {
//     if (const auto hw = hwnd(); hw) ::EnableWindow(hw, b ? TRUE : FALSE);
//   }

//   void close_confirmation(bool b) {
//     if (const auto w = _window(); w) w->close_confirmation = b;
//   }

//   void position(int2 Pos) {
//     if (const auto hw = hwnd(); hw) ::SetWindowPos(hw, nullptr, Pos.x, Pos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
//   }

//   void size(int2 Size) {
//     const auto w = _window();
//     if (const auto hw = w ? w->hwnd : HWND(); hw) {
//       const auto width = Size.x + w->margin.z, height = Size.y + w->margin.w;
//       ::SetWindowPos(hw, nullptr, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
//     }
//   }

//   void cursor(int2 Pos) {
//     const auto w = _window();
//     if (const auto hw = w ? w->hwnd : HWND(); hw) {
//       POINT p{Pos.x, Pos.y};
//       ::ClientToScreen(hw, &p);
//       ::SetCursorPos(p.x, p.y);
//       window_class.cursor_pos = {p.x, p.y};
//       w->cursor_pos = Pos;
//     }
//   }

//   std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
//     if (const auto w = _window(); !w) return unexpected_error(errors::not_initialized, "window is not initialized");
//     else if (auto& rt = w->rendertarget; rt) return rt.begin_draw(src);
//     else return unexpected_error(errors::not_initialized, "window render target is not initialized");
//   }

//   std::expected<drawing, error_trace> begin_draw(const color& clear_color, const source& src = {}) {
//     if (const auto w = _window(); !w) return unexpected_error(errors::not_initialized, "window is not initialized");
//     else if (auto& rt = w->rendertarget; rt) return rt.begin_draw(clear_color, src);
//     else return unexpected_error(errors::not_initialized, "window render target is not initialized");
//   }
// };

// inline std::expected<window::slave, error_trace> window::master::open_subwindow(
//   int2 offset, int2 size, null_terminated<wchar_t> title, window_style style, bool hidden) {
//   auto mw = window_class.windows.get(_master_id);
//   if (!mw) return unexpected_error(errors::invalid_argument, "master window is not valid");
//   switch (style) {
//   case window_style::unknown: style = mw->style; break;
//   case window_style::regular:
//   case window_style::fixed:
//   case window_style::borderless: break;
//   default: return unexpected_error(errors::invalid_argument, "invalid window style");
//   }
//   std::unique_ptr<window_slot> sw = std::make_unique<window_slot>();
//   auto res = sw->initialize(mw->position() + offset, size, title.data(), style, hidden, false, true);
//   if (!res) return unexpected_error(res.error());
//   const auto slave_hwnd = sw->hwnd;
//   const auto slave_id = mw->slaves.push(std::move(sw));
//   constexpr int off = sizeof(slave_id.index);
//   ::SetWindowLongPtrW(slave_hwnd, off * 0, _master_id.index);
//   ::SetWindowLongPtrW(slave_hwnd, off * 1, _master_id.generation);
//   ::SetWindowLongPtrW(slave_hwnd, off * 2, slave_id.index);
//   ::SetWindowLongPtrW(slave_hwnd, off * 3, slave_id.generation);
//   return slave(_master_id, slave_id);
// }

// //////////////////////////////////////// MARK: control::slot

// class controls::control::slot {
//   slot(const slot&) = delete;
//   slot& operator=(const slot&) = delete;

// protected:
//   slotlist<window_slot>::id master_id;
//   slotlist<window_slot>::id slave_id;
//   slotlist<slot>::id control_id;

//   window_slot* _window() const noexcept {
//     const auto master = window_class.windows.get(master_id);
//     return !master || slave_id == slotlist<window_slot>::id() ? master : master->slaves.get(slave_id);
//   }

// public:
//   float2 position{}, size{}, radius{}, padding{};
//   color background_color = colors::white, border_color = colors::black;
//   float border_width = 1.0f;
//   bool visible = true, enabled = true;

//   virtual ~slot() noexcept = default;
//   slot() noexcept = default;
//   slot(slot&&) noexcept = default;
//   slot& operator=(slot&&) noexcept = default;

//   bool hit_test(float2 pt) const {
//     return pt.x >= position.x && pt.x <= position.x + size.x && pt.y >= position.y && pt.y <= position.y + size.y;
//   }

//   virtual std::expected<void, error_trace> draw() const {
//     if (auto res = fill_round_rectangle(position, size, radius, background_color); !res)
//       return unexpected_error(res.error());
//     if (auto res = draw_round_rectangle(position, size, radius, border_color, border_width); !res)
//       return unexpected_error(res.error());
//     return {};
//   }

//   virtual std::expected<bool, error_trace> proc(const MSG& msg) { return {}; }
// };

// //////////////////////////////////////// MARK: control methods

// inline window_slot* controls::control::_window() const noexcept {
//   const auto master = window_class.windows.get(_master_id);
//   return master && !_slave_id.is_zero() ? master->slaves.get(_slave_id) : master;
// }

// inline controls::control::slot* controls::control::_control() const noexcept {
//   const auto w = _window();
//   return w ? w->controls.get(_control_id) : nullptr;
// }

// template<typename Ctrl, typename Slot, typename W>
// inline std::expected<Ctrl, error_trace> controls::control::_add(W& Window) {
//   if (!Window) return unexpected_error(errors::invalid_argument, "window is not valid");
//   const auto hwnd = Window.hwnd();
//   slotlist<window_slot>::id master_id, slave_id;
//   constexpr int offset = sizeof(master_id.index);
//   master_id.index = size_t(::GetWindowLongPtrW(hwnd, offset * 0));
//   master_id.generation = size_t(::GetWindowLongPtrW(hwnd, offset * 1));
//   auto w = window_class.windows.get(master_id);
//   if (!w) return unexpected_error(errors::invalid_argument, "master window is not valid");
//   if (w->is_slave) {
//     slave_id.index = size_t(::GetWindowLongPtrW(hwnd, offset * 2));
//     slave_id.generation = size_t(::GetWindowLongPtrW(hwnd, offset * 3));
//     w = w->slaves.get(slave_id);
//     if (!w) return unexpected_error(errors::invalid_argument, "slave window is not valid");
//   }
//   std::unique_ptr<Slot> control_slot = std::make_unique<Slot>();
//   const auto control_id = w->controls.push(std::move(control_slot));
//   return Ctrl(master_id, slave_id, control_id);
// }

// inline controls::control::~control() noexcept { this->remove(); }
// inline controls::control::control() noexcept : _master_id(), _slave_id(), _control_id() {}

// inline controls::control::control(const slotlist<window_slot>::id& master_id, const slotlist<window_slot>::id& slave_id,
//   const slotlist<control::slot>::id& control_id) noexcept
//   : _master_id(master_id), _slave_id(slave_id), _control_id(control_id) {}

// inline controls::control::control(controls::control&& other) noexcept
//   : _master_id(std::exchange(other._master_id, {})), _slave_id(std::exchange(other._slave_id, {})),
//     _control_id(std::exchange(other._control_id, {})) {}

// inline controls::control& controls::control::operator=(controls::control&& other) noexcept {
//   if (this == &other) return *this;
//   this->remove();
//   _master_id = std::exchange(other._master_id, {});
//   _slave_id = std::exchange(other._slave_id, {});
//   _control_id = std::exchange(other._control_id, {});
//   return *this;
// }

// inline controls::control::operator bool() const noexcept {
//   const auto w = _window();
//   return w ? w->controls.contains(_control_id) : false;
// }

// inline void controls::control::remove() noexcept {
//   if (const auto w = _window(); w) w->controls.erase(_control_id);
// }

// inline controls::control::slot* controls::control::operator->() const noexcept { return _control(); }

// inline std::expected<controls::control, error_trace> controls::control::add(auto& window) {
//   return _add<controls::control, controls::control::slot>(window);
// }

// //////////////////////////////////////// MARK: mainloop

// inline bool mainloop() {
//   ++window_class.frame_count;
//   for (auto& mb : window_class.windows) {
//     if (auto d = mb.rendertarget.begin_draw(); !d) {
//       window_class.last_error = d.error().push();
//       break;
//     } else
//       for (auto& cb : mb.controls)
//         if (cb.visible) cb.draw();
//     mb.swapchain->Present(1, 0);
//     for (auto& sb : mb.slaves) {
//       if (auto d = sb.rendertarget.begin_draw(); !d) {
//         window_class.last_error = d.error().push();
//         break;
//       }
//       for (auto& cb : sb.controls)
//         if (cb.visible) cb.draw();
//       sb.swapchain->Present(1, 0);
//     }
//   }
//   for (MSG msg; ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);) {
//     if (msg.message == WM_QUIT) return false;
//     if (window_class.last_error.error.code != errors::success) return false;
//     ::TranslateMessage(&msg);
//     auto self = reinterpret_cast<window_slot*>(::GetWindowLongPtrW(msg.hwnd, GWLP_USERDATA));
//     if (self) {
//       POINT pt = msg.pt;
//       ::ScreenToClient(msg.hwnd, &msg.pt);
//       std::expected<bool, error_trace> control_result;
//       for (auto& cb : self->controls | std::ranges::views::reverse) {
//         if (cb.enabled && cb.visible && cb.hit_test(float2(msg.pt.x, msg.pt.y))) {
//           control_result = cb.proc(msg);
//           break;
//         }
//       }
//       if (!control_result) {
//         window_class.last_error = control_result.error().push();
//         break;
//       } else if (*control_result) continue; // message has been handled if control::proc returned true
//     }
//     ::DispatchMessageW(&msg);
//   }
//   { // updates cursor position
//     static_assert(sizeof(POINT) == sizeof(window_class.cursor_pos));
//     ::GetCursorPos((POINT*)&window_class.cursor_pos);
//     for (auto& mb : window_class.windows) {
//       mb.cursor_pos = window_class.cursor_pos;
//       ::ScreenToClient(mb.hwnd, (POINT*)&mb.cursor_pos);
//       for (auto& sb : mb.slaves) ::ScreenToClient(sb.hwnd, (POINT*)&sb.cursor_pos);
//     }
//   }
//   return window_class.last_error.error.code == errors::success;
// }

// //////////////////////////////////////// MARK: window procedures

// inline LRESULT __stdcall decltype(window_class)::proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
//   struct ids {
//     slotlist<window_slot>::id master_id{};
//     slotlist<window_slot>::id slave_id{};
//     slotlist<controls::control::slot>::id control_id{};
//   };
//   auto self = reinterpret_cast<window_slot*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
//   if (!self) return ::DefWindowProcW(hwnd, msg, wparam, lparam);
//   switch (msg) {
//   case WM_SIZE: {
//     const auto width = LOWORD(lparam), height = HIWORD(lparam);
//     if (auto res = self->resize_rendertarget(uint2(width, height)); !res) window_class.last_error = res.error().push();
//     return 0;
//   }
//   case WM_CLOSE:
//     if (self->close_confirmation && ::MessageBoxW(hwnd, L"Close window?", L"Confirmation", MB_YESNO) == IDNO) return 0;
//     return ::DestroyWindow(hwnd), 0;
//   case WM_NCDESTROY:
//     slotlist<window_slot>::id master_id;
//     constexpr int offset = sizeof(master_id.index);
//     master_id.index = size_t(::GetWindowLongPtrW(hwnd, offset * 0));
//     master_id.generation = size_t(::GetWindowLongPtrW(hwnd, offset * 1));
//     if (self->is_slave) {
//       slotlist<window_slot>::id slave_id;
//       slave_id.index = size_t(::GetWindowLongPtrW(hwnd, offset * 2));
//       slave_id.generation = size_t(::GetWindowLongPtrW(hwnd, offset * 3));
//       if (const auto master = window_class.windows.get(master_id); master) master->slaves.erase(slave_id);
//     } else {
//       std::vector<HWND> slave_handles;
//       slave_handles.reserve(self->slaves.size());
//       for (auto& slave : self->slaves) slave_handles.push_back(slave.hwnd);
//       for (const auto sh : slave_handles) {
//         ::SetWindowLongPtrW(sh, GWLP_USERDATA, 0);
//         ::DestroyWindow(sh);
//       }
//       window_class.windows.erase(master_id);
//       if (window_class.windows.empty()) ::PostQuitMessage(0);
//     }
//     break;
//   }
//   return ::DefWindowProcW(hwnd, msg, wparam, lparam);
// }
// } // namespace yw
