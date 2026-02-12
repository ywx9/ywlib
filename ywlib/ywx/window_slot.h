// #pragma once
// #include "ywx/window_class.h"

// namespace yw {

// namespace window {

// class base;
// class master;
// class slave;
// inline std::expected<master, error_trace> open(
//   int2 pos, int2 size, null_terminated<wchar_t> title, style style = style::regular, bool hidden = false);
// inline std::expected<master, error_trace> open(
//   int2 size, null_terminated<wchar_t> title, style style = style::regular, bool hidden = false);
// } // namespace window

// class window_slot {
//   friend class window::base;
//   friend class window::master;
//   friend class window::slave;
//   friend std::expected<window::master, error_trace> window::open(
//     int2, int2, null_terminated<wchar_t>, style, bool);
//   friend std::expected<window::master, error_trace> window::open(int2, null_terminated<wchar_t>, style, bool);

//   HWND _hwnd{};
//   style _style{};
//   int4 _margin{};
//   bitmap _rendertarget;
//   comptr<IDXGISwapChain1> _swapchain;
//   stopwatch _timer;
//   int2 _cursor_pos{};
//   bool _is_slave = false;

//   slotlist<window_slot> _slaves;
//   slotlist<control_slot> _controls;

//   window_slot(const window_slot&) = delete;
//   window_slot& operator=(const window_slot&) = delete;

//   std::expected<void, error_trace> _create_window(const wchar_t* t, style s) {
//     switch (this->_style = s) {
//     case style::regular:
//     case style::fixed:
//     case style::borderless: break;
//     default: return unexpected_error(errors::invalid_argument, "invalid window style");
//     }
//     _hwnd = ::CreateWindowExW(
//       WS_EX_ACCEPTFILES, window_class.name.data(), t, DWORD(s), 0, 0, 0, 0, 0, 0, window_class.hinstance, 0);
//     if (!_hwnd) return unexpected_win32_error("CreateWindowExW failed");
//     ::SetWindowLongPtrW(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
//     RECT cr{}, wr{};
//     if (!::GetClientRect(_hwnd, &cr)) return unexpected_win32_error("GetClientRect failed");
//     if (!::GetWindowRect(_hwnd, &wr)) return unexpected_win32_error("GetWindowRect failed");
//     const auto left = (wr.right - wr.left - cr.right) / 2;
//     const auto top = wr.bottom - wr.top - cr.bottom - left;
//     _margin = int4(left, top, 2 * left, left + top);
//     return {};
//   }

//   std::expected<void, error_trace> _set_possize(int2 p, int2 s) {
//     if (!::SetWindowPos(_hwnd, nullptr, p.x, p.y, s.x + _margin.z, s.y + _margin.w, SWP_NOZORDER))
//       return unexpected_win32_error("SetWindowPos failed");
//     else return {};
//   }

//   std::expected<void, error_trace> _set_possize(int2 s) {
//     if (HWND desktop; !(desktop = ::GetDesktopWindow())) return unexpected_win32_error("GetDesktopWindow failed");
//     else if (RECT r; !::GetClientRect(desktop, &r)) return unexpected_win32_error("GetClientRect failed");
//     else return _set_possize(int2((r.right - s.x - _margin.z) / 2, (r.bottom - s.y - _margin.w) / 2), s);
//   }

//   std::expected<void, error_trace> _resize_rendertarget(uint2 size) {
//     if (_swapchain) {
//       _rendertarget = {};
//       if (auto hr = _swapchain->ResizeBuffers(0, size.x, size.y, DXGI_FORMAT_UNKNOWN, 0); FAILED(hr))
//         return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int32_t(hr));
//     } else {
//       if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
//       auto desc = DXGI_SWAP_CHAIN_DESC1(size.x, size.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
//       desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
//       auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), _hwnd, &desc, nullptr, nullptr, &_swapchain.get());
//       if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChainForHwnd failed", int32_t(hr));
//     }
//     if (auto res = bitmap::create(_swapchain.get()); !res) return unexpected_error(res.error());
//     else _rendertarget = std::move(*res);
//     return {};
//   }

//   std::expected<void, error_trace> _initialize(
//     int2 pos, int2 size, const wchar_t* title, window_style style, bool hidden, bool nopos, bool slave) {
//     if (*this) return unexpected_error(errors::invalid_operation, "window is already initialized");
//     this->_is_slave = slave;
//     if (auto res = window_class.initialize(); !res) return unexpected_error(res.error());
//     if (auto res = _create_window(title, style); !res) return unexpected_error(res.error());
//     if (auto res = nopos ? _set_possize(size) : _set_possize(pos, size); !res) return unexpected_error(res.error());
//     if (auto res = _resize_rendertarget(uint2(size.x, size.y)); !res) return unexpected_error(res.error());
//     if (!hidden) ::ShowWindow(_hwnd, SW_SHOW), ::SetForegroundWindow(_hwnd), ::SetActiveWindow(_hwnd);
//     return {};
//   }

// public:
//   bool close_confirmation = false;

//   ~window_slot() noexcept = default;
//   window_slot() noexcept = default;
//   window_slot(window_slot&&) noexcept = default;
//   window_slot& operator=(window_slot&&) noexcept = default;

//   void close() noexcept {
//     if (_hwnd) ::DestroyWindow(std::exchange(_hwnd, nullptr));
//   }

//   explicit operator bool() const noexcept { return _hwnd != nullptr; }
//   HWND hwnd() const noexcept { return _hwnd; }
//   window_style style() const noexcept { return _style; }
//   int4 margin() const noexcept { return _margin; }
//   bool is_slave() const noexcept { return _is_slave; }

//   int2 position() const noexcept {
//     if (!_hwnd) return {};
//     if (RECT r; ::GetWindowRect(_hwnd, &r)) return int2(r.left, r.top);
//     else return {};
//   }

//   int2 size() const noexcept {
//     if (!_hwnd) return {};
//     if (RECT r; ::GetClientRect(_hwnd, &r)) return int2(r.right - r.left, r.bottom - r.top);
//     else return {};
//   }

//   int2 cursor() const noexcept { return _cursor_pos; }

//   void show(bool b) {
//     if (!_hwnd) return;
//     if (b) ::ShowWindow(_hwnd, SW_SHOW), ::SetForegroundWindow(_hwnd), ::SetActiveWindow(_hwnd);
//     else ::ShowWindow(_hwnd, SW_HIDE);
//   }

//   void enable(bool b) {
//     if (!_hwnd) return;
//     if (b) ::EnableWindow(_hwnd, TRUE), _timer.start();
//     else ::EnableWindow(_hwnd, FALSE), _timer.stop();
//   }

//   void position(int2 Pos) {
//     if (_hwnd) ::SetWindowPos(_hwnd, nullptr, Pos.x, Pos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
//   }

//   void size(int2 Size) {
//     if (!_hwnd) return;
//     const auto width = Size.x + _margin.z;
//     const auto height = Size.y + _margin.w;
//     ::SetWindowPos(_hwnd, nullptr, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
//   }

//   void cursor(int2 Pos) {
//     if (!_hwnd) return;
//     _cursor_pos = Pos;
//     ::ClientToScreen(_hwnd, reinterpret_cast<POINT*>(&Pos));
//     ::SetCursorPos(Pos.x, Pos.y);
//     window_class.cursor_pos = Pos;
//   }

//   double elapsed_seconds() const noexcept { return _timer.seconds(); }

//   std::expected<drawing, error_trace> begin_draw(const source& src = {}) { return _rendertarget.begin_draw(src); }

//   std::expected<drawing, error_trace> begin_draw(const color& clear_color, const source& src = {}) {
//     return _rendertarget.begin_draw(clear_color, src);
//   }
// };

// template<typename Window> concept is_window = requires(Window& w) {
//   typename Window::slot_type;
//   requires derived_from<typename Window::slot_type, window_slot>;
//   { w.hwnd() } -> convertible_to<HWND>;
// };
// } // namespace yw
