// #pragma once
// #include "ywx/text_format.h"

// namespace yw {

// inline class {
//   bool _initialized = false;

// public:
//   static constexpr std::wstring_view name = L"ywlib_window";
//   const HINSTANCE hinstance = ::GetModuleHandleW(nullptr);

//   slotlist<window> active_windows{};
//   error_trace last_error{};

//   static LRESULT __stdcall proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

//   std::expected<void, error_trace> initialize() {
//     if (_initialized) return {};
//     if (!hinstance) return unexpected_error(errors::operation_failed, "GetModuleHandleW failed");
//     WNDCLASSW wc{};
//     wc.lpfnWndProc = proc;
//     wc.hInstance = hinstance;
//     wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
//     wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
//     wc.lpszClassName = name.data();
//     if (::RegisterClassW(&wc)) return _initialized = true, std::expected<void, error_trace>();
//     else return unexpected_error(errors::operation_failed, "RegisterClassW failed", int32_t(::GetLastError()));
//   }
// } window_class;

// enum class window_style : uint32_t {
//   unknown,
//   regular = WS_OVERLAPPEDWINDOW,
//   fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
//   borderless = WS_POPUP,
// };

// std::unexpected<error_trace> unexpected_win32_error(const char* msg, const source& src = {}) {
//   return yw::unexpected_error(errors::operation_failed, msg, int32_t(::GetLastError()), {}, src);
// }

// class window;
// class subwindow;

// class window_base {
//   virtual ~window_base() = default;
//   window_base() = default;
//   window_base(const window_base&) = delete;
//   window_base& operator=(const window_base&) = delete;
//   window_base(window_base&&) = delete;
//   window_base& operator=(window_base&&) = delete;

//   std::expected<void, error_trace> _create_window(const wchar_t* t, window_style s) {
//     switch (style = s) {
//     case window_style::regular:
//     case window_style::fixed:
//     case window_style::borderless: break;
//     default: return unexpected_error(errors::invalid_argument, "invalid window style");
//     }
//     if (hwnd = ::CreateWindowExW(0, window_class.name.data(), t, static_cast<DWORD>(s),
//       0, 0, 0, 0, nullptr, nullptr, window_class.hinstance, this)) return {};
//     else return unexpected_win32_error("CreateWindowExW failed");
//   }
//   std::expected<void, error_trace> _calculate_padding() {
//     RECT client_rect{}, window_rect{};
//     if (!::GetClientRect(hwnd, &client_rect)) return unexpected_win32_error("GetClientRect failed");
//     if (!::GetWindowRect(hwnd, &window_rect)) return unexpected_win32_error("GetWindowRect failed");
//     const auto left = (window_rect.right - window_rect.left - client_rect.right) / 2;
//     const auto top = window_rect.bottom - window_rect.top - client_rect.bottom - left;
//     return pad = int4(left, top, 2 * left, top + left), std::expected<void, error_trace>();
//   }
//   std::expected<void, error_trace> _set_sizepos(int2 s, int2 p) {
//     if (::SetWindowPos(hwnd, nullptr, p.x, p.y, s.x + pad.z, s.y + pad.w, SWP_NOZORDER | SWP_NOACTIVATE)) return {};
//     else return unexpected_win32_error("SetWindowPos failed");
//   }
//   std::expected<void, error_trace> _set_sizepos(int2 s) {
//     if (auto desktop = ::GetDesktopWindow(); !desktop) return unexpected_win32_error("GetDesktopWindow failed");
//     else if (RECT rect; !::GetClientRect(desktop, &rect)) return unexpected_win32_error("GetClientRect failed");
//     else return _set_sizepos(int2((rect.right - s.x) / 2, (rect.bottom - s.y) / 2), s);
//   }
//   void _show() { ::ShowWindow(hwnd, SW_SHOW), ::SetForegroundWindow(hwnd), ::SetActiveWindow(hwnd); }

// protected:
//   HWND hwnd{};
//   window_style style{};
//   int4 pad{};
//   bitmap rendertarget{};
//   comptr<::IDXGISwapChain1> swapchain{};
//   slotlist<window>::id id{};
//   stopwatch timer{};
// public:
// };

// template<derived_from<window_base> T> class window_handle {
//   slotlist<window>::id _id{};
//   slotlist<subwindow>::id _sub_id{};
// public:
// };

// class window final : public window_base {
//   slotlist<subwindow> subwindows{};
// public:
//   static std::expected<window_handle<window>, error_trace> open(int2 pos, uint2 size, null_terminated<wchar_t> title,
//     window_style style = window_style::regular, bool hidden = false);
//   static std::expected<window_handle<window>, error_trace> open(uint2 size, null_terminated<wchar_t> title,
//     window_style style = window_style::regular, bool hidden = false);
// };

// class subwindow : public window_base {
//   slotlist<subwindow>::id sub_id{};
// public:
//   static std::expected<window_handle<subwindow>, error_trace> open(const window_handle<window>& main, int2 pos, uint2 size,
//     null_terminated<wchar_t> title, subwindow_style style = subwindow_style::subwindow, bool hidden = false);
// };

// }

// // namespace yw {

// // class window;
// // class window_handle;
// // class subwindow_handle;

// // class control {};

// // //////////////////////////////////////// MARK: windowclass

// // inline class {
// //   bool _initialized{false};

// // public:
// //   const null_terminated<wchar_t> name = L"ywlib_window";
// //   const HINSTANCE hinstance = ::GetModuleHandleW(nullptr);

// //   slotlist<window> active_windows{};
// //   error_trace last_error{error()};

// //   static LRESULT __stdcall proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

// //   std::expected<void, error_trace> initialize() {
// //     if (_initialized) return {};
// //     if (!hinstance) return unexpected_error(errors::not_initialized, "window: instance handle is null");
// //     WNDCLASSW wc{};
// //     wc.lpfnWndProc = proc;
// //     wc.hInstance = hinstance;
// //     wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
// //     wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
// //     wc.lpszClassName = name.data();
// //     if (::RegisterClassW(&wc)) return _initialized = true, std::expected<void, error_trace>();
// //     else return unexpected_error(errors::operation_failed, "RegisterClassW failed", int32_t(::GetLastError()));
// //   };
// // } windowclass;

// // //////////////////////////////////////// MARK: windowstyle

// // enum class windowstyle : uint32_t {
// //   unknown,
// //   regular = WS_OVERLAPPEDWINDOW,
// //   fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
// //   borderless = WS_POPUP,
// // };

// // //////////////////////////////////////// MARK: windowbase

// // class windowbase {
// // protected:
// // public:
// //   HWND hwnd = nullptr;
// //   windowstyle style = windowstyle::unknown;
// //   virtual ~windowbase() = default;
// //   windowbase() = default;
// // };

// // }

// // ///////////////////////////////////////// MARK: window_style

// // enum class window_style : uint32_t {
// //   unknown,
// //   regular = WS_OVERLAPPEDWINDOW,
// //   fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
// //   borderless = WS_POPUP,
// // };

// // enum class subwindow_style : uint32_t {
// //   unknown,
// //   subwindow, // ex) secondary windows
// //   modal,     // ex) message box
// //   temporary, // ex) context menu
// // };

// // //////////////////////////////////////// MARK: window

// // class window {
// //   ~window() = default;
// //   window() = default;
// //   window(const window&) = delete;
// //   window& operator=(const window&) = delete;
// //   window(window&&) = delete;
// //   window& operator=(window&&) = delete;

// //   std::expected<void, error_trace> _create_window(const wchar_t* Title, window_style Style) {
// //     switch (style = Style) {
// //     case window_style::regular:
// //     case window_style::fixed:
// //     case window_style::borderless: break;
// //     default: return unexpected_error(errors::invalid_argument, "invalid window style");
// //     }
// //     constexpr int _temp_size = 400;
// //     if (hwnd = ::CreateWindowExW(0, window_class.name.data(), Title, static_cast<DWORD>(Style), 0, 0,
// //       _temp_size, _temp_size, nullptr, nullptr, window_class.hinstance, this)) return {};
// //     else  return unexpected_error(errors::operation_failed, "CreateWindowExW failed", int32_t(::GetLastError()));
// //   }

// //   std::expected<void, error_trace> _calculate_padding() {
// //     RECT client_rect{}, window_rect{};
// //     if (!::GetClientRect(hwnd, &client_rect))
// //       return unexpected_error(errors::operation_failed, "GetClientRect failed", int32_t(::GetLastError()));
// //     if (!::GetWindowRect(hwnd, &window_rect))
// //       return unexpected_error(errors::operation_failed, "GetWindowRect failed", int32_t(::GetLastError()));
// //     const auto left = (window_rect.right - window_rect.left - client_rect.right) / 2;
// //     const auto top = window_rect.bottom - window_rect.top - client_rect.bottom - left;
// //     return pad = int4(left, top, 2 * left, top + left), std::expected<void, error_trace>();
// //   }

// //   std::expected<void, error_trace> _set_sizepos(int2 Pos, int2 Size) {
// //     if (!::SetWindowPos(hwnd, nullptr, Pos.x, Pos.y, Size.x + pad.z, Size.y + pad.w, SWP_NOZORDER | SWP_NOACTIVATE))
// //       return unexpected_error(errors::operation_failed, "SetWindowPos failed", int32_t(::GetLastError()));
// //     return {};
// //   }

// //   std::expected<void, error_trace> _set_sizepos(int2 Size) {
// //     if (HWND desktop; !(desktop = ::GetDesktopWindow()))
// //       return unexpected_error(errors::operation_failed, "GetDesktopWindow failed", int32_t(::GetLastError()));
// //     else if (RECT rect; !::GetClientRect(desktop, &rect))
// //       return unexpected_error(errors::operation_failed, "GetClientRect failed", int32_t(::GetLastError()));
// //     else return _set_sizepos(int2((rect.right - Size.x) / 2, (rect.bottom - Size.y) / 2), Size);
// //   }

// //   void _show() {
// //     ::ShowWindow(hwnd, SW_SHOW);
// //     ::SetForegroundWindow(hwnd);
// //     ::SetActiveWindow(hwnd);
// //   }

// // public:
// //   HWND hwnd;
// //   window_style style;
// //   subwindow_style sub_style;
// //   int4 pad;
// //   bitmap rendertarget;
// //   comptr<::IDXGISwapChain1> swapchain;
// //   slotlist<window>::id id;
// //   slotlist<subwindow>::id sub_id;
// //   stopwatch timer;

// //   bool is_subwindow() const noexcept { return sub_id == slotlist<subwindow>::id{}; }

// //   static std::expected<window_handle, error_trace> open(int2 pos, uint2 size, null_terminated<wchar_t> title,
// //     window_style style = window_style::regular, bool hidden = false);

// //   static std::expected<window_handle, error_trace> open(
// //     uint2 size, null_terminated<wchar_t> title, window_style style = window_style::regular, bool hidden = false);
// // };

// // //////////////////////////////////////// MARK: window_handle

// // class window_handle {
// //   friend class window;
// //   slotlist<window>::id _id{};

// //   window_handle(slotlist<window>::id id) noexcept : _id(id) {}
// //   window_handle(const window_handle&) = delete;
// //   window_handle& operator=(const window_handle&) = delete;

// //   window* _window() const { return window_class.active_windows.get(_id); }
// //   HWND _hwnd(window* w) const { return w ? w->hwnd : nullptr; }
// //   std::unexpected<error_trace> _unexpected_error(const source& src = {}) const {
// //     if (_id != slotlist<window>::id{}) return unexpected_error(errors::invalid_operation, "window not found", {}, {}, src);
// //     else return unexpected_error(errors::not_initialized, "window handle not initialized", {}, {}, src);
// //   }
// // public:
// //   window_handle() noexcept = default;
// //   window_handle(window_handle&& wh) noexcept : _id(std::exchange(wh._id, slotlist<window>::id{})) {}
// //   window_handle& operator=(window_handle&& wh) noexcept { return _id = wh._id, wh._id = slotlist<window>::id{}, *this; }
// //   ~window_handle() {
// //     if (const auto hwnd = _hwnd(_window()); hwnd) ::DestroyWindow(hwnd);
// //   }
// //   explicit operator bool() const noexcept { return _window() != nullptr; }

// //   window_style style() {
// //     const auto window = this->_window();
// //     return window ? window->style : window_style::unknown;
// //   }

// //   std::expected<void, error_trace> enable() {
// //     if (const auto window = this->_window(); !window) return _unexpected_error();
// //     else if (const auto hwnd = _hwnd(window); hwnd) {
// //       window->timer.start();
// //       ::EnableWindow(hwnd, TRUE);
// //       return {};
// //     } else return _unexpected_error();
// //   }

// //   std::expected<void, error_trace> disable() {
// //     if (const auto window = this->_window(); !window) return _unexpected_error();
// //     else if (const auto hwnd = _hwnd(window); hwnd) {
// //       window->timer.stop();
// //       ::EnableWindow(hwnd, FALSE);
// //       return {};
// //     } else return _unexpected_error();
// //   }

// //   std::expected<void, error_trace> show() {
// //     if (const auto hwnd = _hwnd(_window()); hwnd) {
// //       ::ShowWindow(hwnd, SW_SHOW);
// //       ::SetForegroundWindow(hwnd);
// //       ::SetActiveWindow(hwnd);
// //       return {};
// //     } else return _unexpected_error();
// //   }

// //   std::expected<void, error_trace> hide() {
// //     if (const auto hwnd = _hwnd(_window()); hwnd)
// //       return ::ShowWindow(hwnd, SW_HIDE), std::expected<void, error_trace>{};
// //     else return _unexpected_error();
// //   }

// //   std::expected<uint2, error_trace> size() {
// //     if (const auto hwnd = _hwnd(_window()); hwnd) {
// //       if (RECT rect{}; ::GetClientRect(hwnd, &rect)) return uint2(rect.right, rect.bottom);
// //       else return unexpected_error(errors::operation_failed, "GetClientRect failed", int32_t(::GetLastError()));
// //     } else return _unexpected_error();
// //   }

// //   std::expected<void, error_trace> size(uint2 Size) {
// //     const auto window = this->_window();
// //     if (const auto hwnd = _hwnd(window); hwnd) {
// //       const auto w = Size.x + window->pad.z, h = Size.y + window->pad.w;
// //       if (::SetWindowPos(hwnd, nullptr, 0, 0, w, h, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE)) return {};
// //       else return unexpected_error(errors::operation_failed, "SetWindowPos failed", int32_t(::GetLastError()));
// //     } else return _unexpected_error();
// //   }

// //   std::expected<int2, error_trace> position() {
// //     if (const auto hwnd = _hwnd(_window()); hwnd) {
// //       if (RECT rect{}; ::GetWindowRect(hwnd, &rect)) return int2{rect.left, rect.top};
// //       else return unexpected_error(errors::operation_failed, "GetWindowRect failed", int32_t(::GetLastError()));
// //     } else return _unexpected_error();
// //   }

// //   std::expected<void, error_trace> position(int2 Pos) {
// //     if (const auto hwnd = _hwnd(_window()); hwnd) {
// //       if (::SetWindowPos(hwnd, nullptr, Pos.x, Pos.y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE)) return {};
// //       else return unexpected_error(errors::operation_failed, "SetWindowPos failed", int32_t(::GetLastError()));
// //     } else return _unexpected_error();
// //   }

// //   std::expected<void, error_trace> close() {
// //     if (const auto window = _window(); window) {
// //       if (::DestroyWindow(window->hwnd)) return {};
// //       else return unexpected_error(errors::operation_failed, "DestroyWindow failed", int32_t(::GetLastError()));
// //     } else return _unexpected_error();
// //   }

// //   std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
// //     if (const auto window = _window(); window) {
// //       return window->rendertarget.begin_draw(src);
// //     } else return _unexpected_error();
// //   }

// //   std::expected<drawing, error_trace> begin_draw(const color& clear_color, const source& src = {}) {
// //     if (const auto window = _window(); window) {
// //       return window->rendertarget.begin_draw(clear_color, src);
// //     } else return _unexpected_error();
// //   }

// //   std::expected<subwindow_handle, error_trace> open_subwindow(int2 pos, uint2 size, null_terminated<wchar_t> title,
// //     subwindow_style style = subwindow_style::subwindow, bool hidden = false);
// // };

// // /////////////////////////////////////// MARK: window::open

// // inline std::expected<window_handle, error_trace> window::open(int2 pos, uint2 size, null_terminated<wchar_t> title,
// //     window_style style = window_style::regular, bool hidden = false) {
// //   if (auto res = window_class.initialize(); !res) return unexpected_error(res.error());
// //   std::unique_ptr<window> wnd = std::make_unique<window>();
// //   if (auto res = wnd->_create_window(title.data(), style); !res) return unexpected_error(res.error());
// //   if (auto res = wnd->_calculate_padding(); !res) return unexpected_error(res.error());
// //   if (auto res = wnd->_set_sizepos(pos, int2(size)); !res) return unexpected_error(res.error());
// //   if (!hidden) wnd->_show();
// //   const auto id = window_class.active_windows.insert(std::move(*wnd));
// //   return window_handle(id);
// // }

// // inline std::expected<window_handle, error_trace> window::open(
// //   uint2 size, null_terminated<wchar_t> title, window_style style = window_style::regular, bool hidden = false) {
// //   if (auto res = window_class.initialize(); !res) return unexpected_error(res.error());
// //   std::unique_ptr<window> wnd = std::make_unique<window>();
// //   if (auto res = wnd->_create_window(title.data(), style); !res) return unexpected_error(res.error());
// //   if (auto res = wnd->_calculate_padding(); !res) return unexpected_error(res.error());
// //   if (auto res = wnd->_set_sizepos(int2(size)); !res) return unexpected_error(res.error());
// //   if (!hidden) wnd->_show();
// //   const auto id = window_class.active_windows.insert(std::move(*wnd));
// //   return window_handle(id);
// // }

// // //////////////////////////////////////// MARK: subwindow_handle

// // class subwindow_handle {
// //   friend class window;
// //   friend class window_handle;
// //   slotlist<window>::id _id{}, _main{};

// //   subwindow_handle(slotlist<window>::id id, slotlist<window>::id owner) noexcept : _id(id), _main(owner) {}
// //   subwindow_handle(const subwindow_handle&) = delete;
// //   subwindow_handle& operator=(const subwindow_handle&) = delete;

// //   window* _main() const { return window_class.active_windows.get(_main); }
// //   window* _sub() const { return _main() ? _main()->subwindows.get(_id) : nullptr; }
// //   std::unexpected<error_trace> _unexpected_error(const source& src = {}) const {
// //     if (_main == slotlist<window>::id{}) return unexpected_error(errors::not_initialized, "mainwindow not initialized", {}, {}, src);
// //     if (_id == slotlist<window>::id{}) return unexpected_error(errors::not_initialized, "subwindow not initialized", {}, {}, src);
// //     if (const auto main = _main(); !main) return unexpected_error(errors::invalid_operation, "mainwindow not found", {}, {}, src);
// //     else return unexpected_error(errors::invalid_operation, "subwindow not found", {}, {}, src);
// //   }
// // public:
// //   subwindow_handle() noexcept = default;
// //   subwindow_handle(subwindow_handle&& swh) noexcept
// //     : _id(std::exchange(swh._id, slotlist<window>::id{})), _main(std::exchange(swh._main, slotlist<window>::id{})) {}
// //   subwindow_handle& operator=(subwindow_handle&& swh) noexcept {
// //     if (this == &swh) return *this;
// //     _id = std::exchange(swh._id, slotlist<window>::id{});
// //     _main = std::exchange(swh._main, slotlist<window>::id{});
// //     return *this;
// //   }
// //   ~subwindow_handle() {
// //     if (auto sub = _sub(); sub && sub->hwnd) ::DestroyWindow(hwnd);
// //   }
// //   explicit operator bool() const noexcept { return _sub() != nullptr; }

// //   subwindow_style style() {
// //     return [](window* sub) { return sub ? sub->sub_style : subwindow_style::unknown; }(_sub());
// //   }

// //   std::expected<void, error_trace> show() {
// //     if (const auto main = _main(); !main) return _unexpected_error();
// //     else if (const auto sub = main->subwindows.get(_id); sub) {
// //       if (auto res = main->show(); !res) return res;
// //       if (sub->sub_style == subwindow_style::subwindow)
// //         if (auto res = main->disable(); !res) return res;
// //       ::ShowWindow(sub->hwnd, SW_SHOW);
// //       ::SetForegroundWindow(sub->hwnd);
// //       ::SetActiveWindow(sub->hwnd);
// //       return {};
// //     } else return _unexpected_error();
// //   }

// //   std::expected<void, error_trace> hide() {
// //     if (const auto sub = _sub(); sub && sub->hwnd) {
// //       return ::ShowWindow(sub->hwnd, SW_HIDE), std::expected<void, error_trace>();
// //     } else return _unexpected_error();
// //   }

// //   std::expected<uint2, error_trace> size() {
// //     if (const auto sub = _sub(); sub && sub->hwnd) {
// //       if (RECT rect{}; ::GetClientRect(sub->hwnd, &rect)) return uint2(rect.right, rect.bottom);
// //       else return unexpected_error(errors::operation_failed, "GetClientRect failed", int32_t(::GetLastError()));
// //     } else return _unexpected_error();
// //   }

// //   std::expected<void, error_trace> size(uint2 Size) {
// //     const auto sub = _sub();
// //     if (sub && sub->hwnd) {
// //       const auto w = Size.x + sub->pad.z, h = Size.y + sub->pad.w;
// //       if (::SetWindowPos(sub->hwnd, nullptr, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE)) return {};
// //       else return unexpected_error(errors::operation_failed, "SetWindowPos failed", int32_t(::GetLastError()));
// //     } else return _unexpected_error();
// //   }

// //   std::expected<int2, error_trace> position() {
// //     if (const auto sub = _sub(); sub && sub->hwnd) {
// //       if (RECT rect{}; ::GetWindowRect(sub->hwnd, &rect)) return int2{rect.left, rect.top};
// //       else return unexpected_error(errors::operation_failed, "GetWindowRect failed", int32_t(::GetLastError()));
// //     } else return _unexpected_error();
// //   }

// //   std::expected<void, error_trace> position(int2 Pos) {
// //     const auto sub = _sub();
// //     if (sub && sub->hwnd) {
// //       if (::SetWindowPos(sub->hwnd, nullptr, Pos.x, Pos.y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE)) return {};
// //       else return unexpected_error(errors::operation_failed, "SetWindowPos failed", int32_t(::GetLastError()));
// //     } else return _unexpected_error();
// //   }

// //   std::expected<void, error_trace> close() {
// //     const auto main = _main();
// //     if (const auto sub = _sub(); main && sub) {
// //       if (::DestroyWindow(sub->hwnd)) return {};
// //       else return unexpected_error(errors::operation_failed, "DestroyWindow failed", int32_t(::GetLastError()));
// //     } else return _unexpected_error();
// //   }

// //   std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
// //     if (const auto sub = _sub(); sub) {
// //       return sub->rendertarget.begin_draw(src);
// //     } else return _unexpected_error();
// //   }

// //   std::expected<drawing, error_trace> begin_draw(const color& clear_color, const source& src = {}) {
// //     if (const auto sub = _sub(); sub) {
// //       return sub->rendertarget.begin_draw(clear_color, src);
// //     } else return _unexpected_error();
// //   }
// // };

// // inline std::expected<subwindow_handle, error_trace> window_handle::open_subwindow(int2 pos, uint2 size,
// //   null_terminated<wchar_t> title, subwindow_style style, bool hidden) {
// //   const auto window = this->_window();
// //   if (!window) return unexpected_error(errors::not_initialized, "window not initialized");
// //   std::unique_ptr<window> sub = std::make_unique<window>();
// //   switch (sub->sub_style = style) {
// //   case subwindow_style::subwindow: sub->style = window->style; break;
// //   case subwindow_style::modal: sub->style = window_style::fixed; break;
// //   case subwindow_style::temporary: sub->style = window_style::borderless; break;
// //   default: return unexpected_error(errors::invalid_argument, "invalid subwindow style");
// //   }
// //   if (auto res = sub->_create_window(title.data(), sub->style); !res) return unexpected_error(res.error());
// //   if (auto res = sub->_calculate_padding(); !res) return unexpected_error(res.error());
// //   if (RECT r; !::GetWindowRect(window->hwnd, &r)) {
// //     if (auto res = sub->_set_sizepos(int2(r.left, r.top) + pos, size); !res) return unexpected_error(res.error());
// //   } else return unexpected_error(errors::operation_failed, "GetWindowRect failed", int32_t(::GetLastError()));
// //   if (!hidden) sub->_show();
// // }
// // // class window {
// // //   friend class window_handle;
// // //   friend class subwindow_handle;
// // //   friend bool mainloop();
// // //   friend LRESULT __stdcall decltype(window_class)::proc(HWND, UINT, WPARAM, LPARAM);
// // //   static constexpr int _temp_size = 400;

// // //   window(const window&) = delete;
// // //   window& operator=(const window&) = delete;
// // //   window(window&&) = delete;
// // //   window& operator=(window&&) = delete;

// // // protected:
// // //   HWND _hwnd{};
// // //   window_style _style{};
// // //   int4 _pad{};
// // //   bitmap _rendertarget{};
// // //   comptr<::IDXGISwapChain1> _swapchain{};
// // //   slotlist<window>::id _main_id{};
// // //   slotlist<subwindow>::id _sub_id{};

// // //   std::expected<void, error_trace> _create_window(null_terminated<wchar_t> title, window_style style) {
// // //     if (auto res = window_class.initialize(); !res) return unexpected_error(res.error());
// // //     switch (_style = style) {
// // //     case window_style::regular:
// // //     case window_style::fixed:
// // //     case window_style::borderless: break;
// // //     default: return unexpected_error(errors::invalid_argument, "invalid window style");
// // //     }
// // //     auto hwnd = ::CreateWindowExW(0, window_class.name.data(), title.data(), static_cast<DWORD>(_style), 0, 0,
// // //       _temp_size, _temp_size, nullptr, nullptr, window_class.hinstance, nullptr);
// // //     if (!hwnd) return unexpected_error(errors::operation_failed, "CreateWindowExW failed",
// // //     int32_t(::GetLastError()));
// // //     ::SetWindowLongPtrW(_hwnd = hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
// // //     return {};
// // //   }

// // //   std::expected<void, error_trace> _calculate_padding() {
// // //     RECT client_rect{}, window_rect{};
// // //     if (!::GetClientRect(_hwnd, &client_rect))
// // //       return unexpected_error(errors::operation_failed, "GetClientRect failed", int32_t(::GetLastError()));
// // //     if (!::GetWindowRect(_hwnd, &window_rect))
// // //       return unexpected_error(errors::operation_failed, "GetWindowRect failed", int32_t(::GetLastError()));
// // //     const auto pad = (window_rect.right - window_rect.left - client_rect.right) / 2; // left, right and bottom are
// // //     same const auto pad_top = window_rect.bottom - window_rect.top - client_rect.bottom - pad; _pad = int4{pad,
// // //     pad_top, 2 * pad, pad + pad_top}; return {};
// // //   }

// // //   std::expected<void, error_trace> _resize_d3d(uint2 size) {
// // //     if (_swapchain) {
// // //       _rendertarget = {};
// // //       if (auto hr = _swapchain->ResizeBuffers(0, size.x, size.y, DXGI_FORMAT_UNKNOWN, 0); FAILED(hr))
// // //         return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int32_t(hr));
// // //     } else {
// // //       if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
// // //       auto desc = DXGI_SWAP_CHAIN_DESC1(size.x, size.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
// // //       desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
// // //       auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), _hwnd, &desc, nullptr, nullptr,
// // //       &_swapchain.get()); if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChainForHwnd
// // //       failed", int32_t(hr));
// // //     }
// // //     if (auto res = bitmap::create(_swapchain.get()); !res) return unexpected_error(res.error());
// // //     else _rendertarget = std::move(*res);
// // //     return {};
// // //   }

// // //   std::expected<void, error_trace> _move_to_center() {
// // //     if (auto desktop = ::GetDesktopWindow(); !desktop)
// // //       return unexpected_error(errors::operation_failed, "GetDesktopWindow failed", int32_t(::GetLastError()));
// // //     else if (RECT r; !::GetClientRect(desktop, &r))
// // //       return unexpected_error(errors::operation_failed, "GetClientRect failed", int32_t(::GetLastError()));
// // //     else if (const auto wh = int2(_rendertarget.size()) + int2(_pad.z, _pad.w); !::SetWindowPos(
// // //                _hwnd, nullptr, (r.right - wh.x) / 2, (r.bottom - wh.y) / 2, wh.x, wh.y, SWP_NOZORDER |
// // //                SWP_NOACTIVATE))
// // //       return unexpected_error(errors::operation_failed, "SetWindowPos failed", int32_t(::GetLastError()));
// // //     else return {};
// // //   }

// // // public:
// // //   slotlist<control> controls{};
// // //   slotlist<subwindow> subwindows{};

// // //   window() noexcept = default;
// // //   ~window() { _hwnd ? void(::DestroyWindow(std::exchange(_hwnd, nullptr))) : void(); }
// // //   explicit operator bool() const noexcept { return _hwnd != nullptr; }

// // //   HWND hwnd() const noexcept { return _hwnd; }

// // //   static std::expected<window_handle, error_trace> create(int2 pos, uint2 size, null_terminated<wchar_t> title,
// // //     window_style style = window_style::regular, bool hidden = false);

// // //   static std::expected<window_handle, error_trace> create(
// // //     uint2 size, null_terminated<wchar_t> title, window_style style = window_style::regular, bool hidden = false);
// // // };

// // // //////////////////////////////////////// MARK: window_handle

// // // class window_handle {
// // //   friend class window;
// // //   friend class subwindow;
// // //   slotlist<window>::id _id{};
// // //   window_handle() noexcept = default;
// // //   window_handle(const window_handle&) = delete;
// // //   window_handle& operator=(const window_handle&) = delete;
// // //   window_handle(slotlist<window>::id id) noexcept : _id(id) {
// // //     if (auto w = _get(); w) w->_main_id = id;
// // //   }
// // //   window* _get() const { return window_class.active_windows.get(_id); }

// // // public:
// // //   ~window_handle() {
// // //     if (auto w = _get(); w) window_class.active_windows.erase(_id);
// // //   }
// // //   window_handle(window_handle&& other) noexcept : _id(std::exchange(other._id, {})) {}

// // //   window_handle& operator=(window_handle&& other) noexcept {
// // //     if (this != &other) _id = std::exchange(other._id, {});
// // //     return *this;
// // //   }

// // //   explicit operator bool() const noexcept { return _get() != nullptr; }

// // //   std::expected<void, error_trace> show() {
// // //     if (auto w = _get(); !w) return unexpected_error(errors::invalid_operation, "window not found");
// // //     else w->show();
// // //     return {};
// // //   }

// // //   std::expected<void, error_trace> hide() {
// // //     if (auto w = _get(); !w) return unexpected_error(errors::invalid_operation, "window not found");
// // //     else w->hide();
// // //     return {};
// // //   }

// // //   std::expected<uint2, error_trace> size() const {
// // //     if (auto w = _get(); !w) return unexpected_error(errors::invalid_operation, "window not found");
// // //     else return w->size();
// // //   }

// // //   std::expected<void, error_trace> size(uint2 Size) {
// // //     if (auto w = _get(); !w) return unexpected_error(errors::invalid_operation, "window not found");
// // //     else return w->size(Size);
// // //   }

// // //   std::expected<int2, error_trace> position() const {
// // //     if (auto w = _get(); !w) return unexpected_error(errors::invalid_operation, "window not found");
// // //     else return w->position();
// // //   }

// // //   std::expected<void, error_trace> position(int2 Pos) {
// // //     if (auto w = _get(); !w) return unexpected_error(errors::invalid_operation, "window not found");
// // //     else return w->position(Pos);
// // //   }



// // //   std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
// // //     if (auto w = _get(); !w) return unexpected_error(errors::invalid_operation, "window not found");
// // //     else return w->begin_draw(src);
// // //   }

// // //   std::expected<drawing, error_trace> begin_draw(const color& clear_color, const source& src = {}) {
// // //     if (auto w = _get(); !w) return unexpected_error(errors::invalid_operation, "window not found");
// // //     else return w->begin_draw(clear_color, src);
// // //   }
// // // };

// // // inline std::expected<window_handle, error_trace> window::create(
// // //   int2 pos, uint2 size, null_terminated<wchar_t> title, window_style style, bool hidden) {
// // //   auto w = std::make_unique<window>();
// // //   if (auto res = w->_create_window(title, style); !res) return unexpected_error(res.error());
// // //   if (auto res = w->_calculate_padding(); !res) return unexpected_error(res.error());
// // //   if (auto res = w->size(size); !res) return unexpected_error(res.error());
// // //   if (auto res = w->position(pos); !res) return unexpected_error(res.error());
// // //   if (!hidden) {
// // //     if (auto res = w->show(); !res) return unexpected_error(res.error());
// // //   }
// // //   const auto id = window_class.active_windows.push(std::move(w));
// // //   return window_handle(id);
// // // }

// // // inline std::expected<window_handle, error_trace> window::create(
// // //   uint2 size, null_terminated<wchar_t> title, window_style style, bool hidden) {
// // //   auto w = std::make_unique<window>();
// // //   if (auto res = w->_create_window(title, style); !res) return unexpected_error(res.error());
// // //   if (auto res = w->_calculate_padding(); !res) return unexpected_error(res.error());
// // //   if (auto res = w->size(size); !res) return unexpected_error(res.error());
// // //   if (auto res = w->_move_to_center(); !res) return unexpected_error(res.error());
// // //   if (!hidden) {
// // //     if (auto res = w->show(); !res) return unexpected_error(res.error());
// // //   }
// // //   const auto id = window_class.active_windows.push(std::move(w));
// // //   return window_handle(id);
// // // }

// // // //////////////////////////////////////// MARK: subwindow

// // // class subwindow : public window {
// // // public:
// // //   using window::operator bool;

// // //   static std::expected<subwindow_handle, error_trace> create(const window_handle& parent, int2 pos, uint2 size,
// // //     null_terminated<wchar_t> title, subwindow_style style = subwindow_style::subwindow, bool hidden = false);
// // // };

// // // ///////////////////////////////////////// MARK: subwindow_handle

// // // class subwindow_handle {
// // //   friend class subwindow;
// // //   slotlist<window>::id _main_id{};
// // //   slotlist<subwindow>::id _id{};
// // //   subwindow_handle() noexcept = default;
// // //   subwindow_handle(const subwindow_handle&) = delete;
// // //   subwindow_handle& operator=(const subwindow_handle&) = delete;
// // //   subwindow_handle(slotlist<window>::id main_id, slotlist<subwindow>::id id) noexcept : _main_id(main_id), _id(id) {
// // //     if (auto w = _get(); w) w->_main_id = main_id, w->_sub_id = id;
// // //   }
// // //   subwindow* _get() const {
// // //     if (auto main_win = window_class.active_windows.get(_main_id); !main_win) return nullptr;
// // //     else return main_win->subwindows.get(_id);
// // //   }

// // // public:
// // //   ~subwindow_handle() {
// // //     if (auto mw = window_class.active_windows.get(_main_id); !mw) return;
// // //     else if (auto w = mw->subwindows.get(_id); !w) return;
// // //     else mw->subwindows.erase(_id);
// // //   }

// // //   subwindow_handle(subwindow_handle&& other) noexcept
// // //     : _main_id(std::exchange(other._main_id, {})), _id(std::exchange(other._id, {})) {}

// // //   subwindow_handle& operator=(subwindow_handle&& other) noexcept {
// // //     if (this == &other) return *this;
// // //     _main_id = std::exchange(other._main_id, {});
// // //     _id = std::exchange(other._id, {});
// // //     return *this;
// // //   }

// // //   explicit operator bool() const noexcept { return _get() != nullptr; }

// // //   std::expected<void, error_trace> show() {
// // //     if (auto w = _get(); !w) return unexpected_error(errors::invalid_operation, "subwindow not found");
// // //     else w->show();
// // //     return {};
// // //   }

// // //   std::expected<void, error_trace> hide() {
// // //     if (auto w = _get(); !w) return unexpected_error(errors::invalid_operation, "subwindow not found");
// // //     else w->hide();
// // //     return {};
// // //   }

// // //   std::expected<uint2, error_trace> size() const {
// // //     if (auto w = _get(); !w) return unexpected_error(errors::invalid_operation, "subwindow not found");
// // //     else return w->size();
// // //   }

// // //   std::expected<void, error_trace> size(uint2 Size) {
// // //     if (auto w = _get(); !w) return unexpected_error(errors::invalid_operation, "subwindow not found");
// // //     else return w->size(Size);
// // //   }

// // //   std::expected<int2, error_trace> position() const {
// // //     if (auto w = _get(); !w) return unexpected_error(errors::invalid_operation, "subwindow not found");
// // //     else return w->position();
// // //   }

// // //   std::expected<void, error_trace> position(int2 Pos) {
// // //     if (auto w = _get(); !w) return unexpected_error(errors::invalid_operation, "subwindow not found");
// // //     else return w->position(Pos);
// // //   }

// // //   std::expected<void, error_trace> close() {
// // //     if (auto mw = window_class.active_windows.get(_main_id); !mw)
// // //       return unexpected_error(errors::not_initialized, "parent window not initialized");
// // //     else if (auto w = mw->subwindows.get(_id); !w)
// // //       return unexpected_error(errors::not_initialized, "subwindow not found");
// // //     else mw->subwindows.erase(_id);
// // //     return {};
// // //   }

// // //   std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
// // //     if (auto w = _get(); !w) return unexpected_error(errors::invalid_operation, "subwindow not found");
// // //     else return w->begin_draw(src);
// // //   }

// // //   std::expected<drawing, error_trace> begin_draw(const color& clear_color, const source& src = {}) {
// // //     if (auto w = _get(); !w) return unexpected_error(errors::invalid_operation, "subwindow not found");
// // //     else return w->begin_draw(clear_color, src);
// // //   }
// // // };

// // // inline std::expected<subwindow_handle, error_trace> subwindow::create(const window_handle& parent, int2 pos, uint2
// // // size,
// // //   null_terminated<wchar_t> title, subwindow_style style, bool hidden) {
// // //   auto main_win = parent._get();
// // //   if (!main_win) return unexpected_error(errors::not_initialized, "parent window not initialized");
// // //   auto w = std::make_unique<subwindow>();
// // //   window_style ws;
// // //   switch (style) {
// // //   case subwindow_style::subwindow: ws = window_style::regular; break;
// // //   case subwindow_style::modal: ws = window_style::fixed; break;
// // //   case subwindow_style::temporary: ws = window_style::borderless; break;
// // //   default: return unexpected_error(errors::invalid_argument, "invalid subwindow style");
// // //   }
// // //   if (auto res = w->_create_window(title, ws); !res) return unexpected_error(res.error());
// // //   if (auto res = w->_calculate_padding(); !res) return unexpected_error(res.error());
// // //   if (auto res = w->size(size); !res) return unexpected_error(res.error());
// // //   if (auto res = w->position(pos); !res) return unexpected_error(res.error());
// // //   if (!hidden)
// // //     if (auto res = w->show(); !res) return unexpected_error(res.error());
// // //   const auto id = main_win->subwindows.push(std::move(w));
// // //   return subwindow_handle(parent._id, id);
// // // }

// // // //////////////////////////////////////// MARK: control

// // // // class control {
// // // //   friend class window;
// // // //   control(const control&) = delete;
// // // //   control& operator=(const control&) = delete;

// // // // protected:
// // // //   HWND _owner = nullptr;
// // // //   control(window& owner);

// // // // public:
// // // //   float2 position{}, size{}, padding = float2::fill(1.0f), rounded_radius{};
// // // //   float border_width = 1.0f;
// // // //   color background_color = colors::white, border_color = colors::black;
// // // //   bool visible{true}, enabled{true};

// // // //   virtual ~control();
// // // //   control() noexcept = default;

// // // //   control(control&& other) noexcept
// // // //     : _owner(std::exchange(other._owner, nullptr)), position(other.position), size(other.size),
// // // //     padding(other.padding),
// // // //       rounded_radius(other.rounded_radius), border_width(other.border_width),
// // // //       background_color(other.background_color), border_color(other.border_color), visible(other.visible),
// // // //       enabled(other.enabled) {}

// // // //   control& operator=(control&& other) noexcept {
// // // //     if (this == &other) return *this;
// // // //     _owner = std::exchange(other._owner, nullptr);
// // // //     position = other.position;
// // // //     size = other.size;
// // // //     padding = other.padding;
// // // //     rounded_radius = other.rounded_radius;
// // // //     border_width = other.border_width;
// // // //     background_color = other.background_color;
// // // //     border_color = other.border_color;
// // // //     visible = other.visible;
// // // //     enabled = other.enabled;
// // // //     return *this;
// // // //   }

// // // //   bool hit_test(float2 point) const {
// // // //     return point.x >= position.x && point.x <= position.x + size.x && point.y >= position.y &&
// // // //            point.y <= position.y + size.y;
// // // //   }

// // // //   virtual std::expected<void, error_trace> draw() = 0;
// // // // };

// // // //////////////////////////////////////// MARK: window

// // // // class window : public window_base {
// // // //   friend bool ::yw::mainloop();
// // // //   friend LRESULT __stdcall decltype(window_class)::proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

// // // //   static constexpr int initial_size = 400;
// // // //   inline static slotlist<window> _active_windows{};

// // // // protected:
// // // //   HWND _hwnd{};
// // // //   window_style _style{};
// // // //   int4 _pad{};
// // // //   bitmap _rendertarget{};
// // // //   comptr<::IDXGISwapChain1> _swapchain{};
// // // //   slotlist<control> _controls{};
// // // //   slotlist<subwindow> _subwindows{};

// // // // public:
// // // //   inline static error_trace last_error{error()};

// // // //   window() noexcept = default;
// // // //   window(const window&) = delete;
// // // //   window& operator=(const window&) = delete;
// // // //   window(window&&) = delete;
// // // //   window& operator=(window&&) = delete;

// // // //   virtual ~window() {
// // // //     if (!_hwnd) return;
// // // //     for (auto& subs : _subwindows)
// // // //       if (auto h = _subwindows.get(subs)->_hwnd) ::DestroyWindow(h);
// // // //     ::DestroyWindow(std::exchange(_hwnd, nullptr));
// // // //   }

// // // //   explicit operator bool() const noexcept { return _hwnd != nullptr; }

// // // //   HWND handle() const noexcept { return _hwnd; }
// // // //   window_style style() const noexcept { return _style; }

// // // //   static std::expected<window_handle, error_trace> create(int2 pos, uint2 size, null_terminated<wchar_t> title,
// // // //     window_style style = window_style::regular, bool hidden = false);

// // // //   static std::expected<window_handle, error_trace> create(
// // // //     uint2 size, null_terminated<wchar_t> title, window_style style = window_style::regular, bool hidden = false);
// // // // };

// // // // //////////////////////////////////////// MARK: window_handle

// // // // // class window_handle {
// // // // //   friend class window;
// // // // //   friend class subwindow;
// // // // //   slotlist<window>::id _id{};
// // // // //   window_handle(slotlist<window>::id id) noexcept : _id(id) {}
// // // // //   window* _get() const { return window::_active_windows.get(_id); }

// // // // // public:
// // // // //   ~window_handle() { assume(close()); }
// // // // //   window_handle() noexcept = default;
// // // // //   window_handle(const window_handle&) = delete;
// // // // //   window_handle& operator=(const window_handle&) = delete;
// // // // //   window_handle(window_handle&& other) noexcept : _id(std::exchange(other._id, {})) {}
// // // // //   window_handle& operator=(window_handle&& other) noexcept {
// // // // //     if (this != &other) _id = std::exchange(other._id, {});
// // // // //     return *this;
// // // // //   }

// // // // //   /// shows the window

// // // // //   /// hides the window

// // // // //   /// gets window client size

// // // // //   /// sets window client size

// // // // //   /// gets window position; left-top corner of the window (including non-client area)

// // // // //   /// sets window position; left-top corner of the window (including non-client area)

// // // // //   /// closes the window

// // // // //   /// begins drawing to the window

// // // // // };

// // // // inline std::expected<window_handle, error_trace> window::create(
// // // //   int2 pos, uint2 size, null_terminated<wchar_t> title, window_style style, bool hidden) {
// // // //   std::unique_ptr<window> w = std::make_unique<window>();
// // // //   if (auto res = w->_create_window(std::move(title), style); !res) return unexpected_error(res.error());
// // // //   else if (auto res = w->_calculate_padding(); !res) return unexpected_error(res.error());
// // // //   else if (auto res = w->_resize(size); !res) return unexpected_error(res.error());
// // // //   else if (auto res = w->_move_to(pos); !res) return unexpected_error(res.error());
// // // //   if (!hidden) w->_show();
// // // //   return window_handle{_active_windows.push(std::move(w))};
// // // // }

// // // // inline std::expected<window_handle, error_trace> window::create(
// // // //   uint2 size, null_terminated<wchar_t> title, window_style style, bool hidden) {
// // // //   std::unique_ptr<window> w = std::make_unique<window>();
// // // //   if (auto res = w->_create_window(std::move(title), style); !res) return unexpected_error(res.error());
// // // //   else if (auto res = w->_calculate_padding(); !res) return unexpected_error(res.error());
// // // //   else if (auto res = w->_resize(size); !res) return unexpected_error(res.error());
// // // //   else if (auto res = w->_move_to_center(); !res) return unexpected_error(res.error());
// // // //   if (!hidden) w->_show();
// // // //   return window_handle{_active_windows.push(std::move(w))};
// // // // }

// // // // //////////////////////////////////////// MARK: subwindow

// // // // class subwindow_handle;

// // // // class subwindow : public window {
// // // //   static window_style _select_window_style(window_style parent_style, subwindow_style style) {
// // // //     switch (style) {
// // // //     case subwindow_style::subwindow: return parent_style;
// // // //     case subwindow_style::modal: return window_style::fixed;
// // // //     case subwindow_style::temporary: return window_style::borderless;
// // // //     default: return window_style::unknown;
// // // //     }
// // // //   }

// // // //   window* _owner{};
// // // //   subwindow_style _style{};

// // // // public:
// // // //   ~subwindow() = default;
// // // //   subwindow() noexcept = default;
// // // //   subwindow(const subwindow&) = delete;
// // // //   subwindow& operator=(const subwindow&) = delete;
// // // //   subwindow(subwindow&&) noexcept = delete;
// // // //   subwindow& operator=(subwindow&&) noexcept = delete;

// // // //   /// creates subwindow attached to the given main_window
// // // //   /// \param pos position relative to the main_window's client area
// // // //   static std::expected<subwindow_handle, error_trace> create(window_handle& main_window, int2 pos, uint2 size,
// // // //     null_terminated<wchar_t> title, subwindow_style style = subwindow_style::subwindow, bool hidden = false);
// // // //   //   {
// // // //   //   if (!main_window) return unexpected_error(errors::not_initialized, "main window is not initialized");
// // // //   //   subwindow sw;
// // // //   //   if (const auto ws = _select_window_style(main_window._style, style); ws == window_style::unknown)
// // // //   //     return unexpected_error(errors::invalid_argument, "invalid subwindow style");
// // // //   //   else if (auto res = sw._create_window(std::move(title), ws); !res) return unexpected_error(res.error());
// // // //   //   if (auto res = sw._calculate_padding(); !res) return unexpected_error(res.error());
// // // //   //   if (auto res = sw._resize(size); !res) return unexpected_error(res.error());
// // // //   //   if (auto main_pos = main_window.position(); !main_pos) return unexpected_error(main_pos.error());
// // // //   //   else if (auto res = sw._move_to(*main_pos + pos); !res) return unexpected_error(res.error());
// // // //   //   if (!hidden) sw._show();
// // // //   //   sw._main = main_window._hwnd, sw._style = style;
// // // //   //   main_window._subwindows.push_back(sw._hwnd);
// // // //   //   return std::move(sw);
// // // //   // }
// // // // };

// // // // //////////////////////////////////////// MARK: subwindow_handle

// // // // class subwindow_handle {
// // // //   friend class subwindow;
// // // //   slotlist<subwindow>::id _id{};
// // // //   subwindow_handle(slotlist<subwindow>::id id) noexcept : _id(id) {}
// // // // };

// // // // inline std::expected<subwindow_handle, error_trace> subwindow::create(window_handle& main_window, int2 pos, uint2
// // // // size,
// // // //   null_terminated<wchar_t> title, subwindow_style style, bool hidden) {
// // // //   auto w = main_window._get();
// // // //   if (!w) return unexpected_error(errors::not_initialized, "main window is not initialized");
// // // //   std::unique_ptr<subwindow> sw = std::make_unique<subwindow>();
// // // //   if (const auto ws = _select_window_style(w->style(), style); ws == window_style::unknown)
// // // //     return unexpected_error(errors::invalid_argument, "invalid subwindow style");
// // // //   else if (auto res = sw->_create_window(std::move(title), ws); !res) return unexpected_error(res.error());
// // // //   if (auto res = sw->_calculate_padding(); !res) return unexpected_error(res.error());
// // // //   if (auto res = sw->_resize(size); !res) return unexpected_error(res.error());
// // // //   if (auto main_pos = main_window.position(); !main_pos) return unexpected_error(main_pos.error());
// // // //   else if (auto res = sw->_move_to(*main_pos + pos); !res) return unexpected_error(res.error());
// // // //   if (!hidden) sw->_show();
// // // //   sw->_owner = w, sw->_style = style;
// // // //   return subwindow_handle{w->_subwindows.push(std::move(sw))};
// // // // }

// // // //////////////////////////////////////// MARK: window_class proc

// // // inline LRESULT __stdcall decltype(window_class)::proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
// // //   auto self = reinterpret_cast<window*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
// // //   if (!self) return ::DefWindowProcW(hwnd, msg, wparam, lparam);
// // //   switch (msg) {
// // //   case WM_SIZE: {
// // //     const auto width = LOWORD(lparam), height = HIWORD(lparam);
// // //     if (auto res = self->_resize_d3d(uint2(width, height)); !res) {
// // //       window_class.last_error = res.error().push();
// // //       print_error("Window resize failed", window_class.last_error);
// // //     }
// // //     return 0;
// // //   }
// // //   case WM_NCDESTROY: {
// // //     ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
// // //     self->_hwnd = nullptr; // prevent double destroy
// // //     if (self->_sub_id == slotlist<subwindow>::id{}) window_class.active_windows.erase(self->_main_id);
// // //     else window_class.active_windows.get(self->_main_id)->subwindows.erase(self->_sub_id);
// // //     if (window_class.active_windows.empty()) ::PostQuitMessage(0);
// // //     break;
// // //   }
// // //   }
// // //   return ::DefWindowProcW(hwnd, msg, wparam, lparam);
// // // }

// // // //////////////////////////////////////// MARK: mainloop

// // // inline bool mainloop() {
// // //   for (auto id : window_class.active_windows) {
// // //     if (auto wnd = window_class.active_windows.get(id); wnd) {
// // //       if (wnd->_swapchain) wnd->_swapchain->Present(1, 0);
// // //       for (auto sub_id : wnd->subwindows)
// // //         if (auto subwnd = wnd->subwindows.get(sub_id); subwnd && subwnd->_swapchain) subwnd->_swapchain->Present(1,
// // //         0);
// // //     }
// // //   }
// // //   for (MSG msg; ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);) {
// // //     if (msg.message == WM_QUIT) return false;
// // //     if (window_class.last_error.error.code != errors::success) return false;
// // //     ::TranslateMessage(&msg), ::DispatchMessageW(&msg);
// // //   }
// // //   return true;
// // // }
// // // } // namespace yw
// // }
