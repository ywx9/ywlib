#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")

#include <shellapi.h>
#pragma comment(lib, "shell32.lib")

export namespace yw {

/** \class yw::window
 *  \brief class to create and manage windows
 ******************************************************************************/

class window {
protected:
  HWND _hwnd{};
  vector2<int4> _pad{};
  window(HWND Hwnd) noexcept : _hwnd(Hwnd) {}
public:
  struct message_t {
    HWND hwnd;
    wmessage message;
    nat8 wparam;
    int8 lparam;
    nat4 time;
    vector2<int> pt;
    operator wmessage() const noexcept { return message; }
    void translate() noexcept { ::TranslateMessage(reinterpret_cast<MSG*>(this)); }
    void dispatch() noexcept { ::DispatchMessageW(reinterpret_cast<MSG*>(this)); }
  };
  static_assert(sizeof(message_t) == sizeof(MSG));
  message_t message{};
  window() noexcept = default;
  window(window&& w) noexcept : _hwnd(exchange(w._hwnd, nullptr)), message(w.message), _pad(w._pad) {}
  operator HWND() const noexcept { return _hwnd; }
  explicit operator bool() const noexcept { return _hwnd != nullptr; }
  [[nodiscard]] window(                                                                            //
    const wclass& Class, stringable auto&& Title, style Style, numeric auto&& X, numeric auto&& Y, //
    numeric auto&& Width, numeric auto&& Height, const source& _ = {}) noexcept {
    try {
      if (!Class) throw std::runtime_error("window class is not registered");
      const auto title = cvt<cat1>(Title);
      const bool visible = bool(Style & style::ws_visible);
      const int size = 500;
      _hwnd = ::CreateWindowExA(int(nat8(Style) >> 32), Class.name.data(), title.data(), int((nat(Style) & 0xEfffffff)), //
                                0, 0, size, size, nullptr, nullptr, main::hinstance, nullptr);
      if (!_hwnd) throw std::runtime_error("CreateWindowEx failed");
      [&](RECT r) { ::GetClientRect(_hwnd, &r), _pad.x = (size - r.right) / 2, _pad.y = size - r.bottom - _pad.x; }({});
      ::SetWindowPos(_hwnd, 0, int(X), int(Y), int(Width) + _pad.x * 2, int(Height) + _pad.x + _pad.y, 0);
      if (visible) ::ShowWindow(_hwnd, SW_SHOW), ::SetFocus(_hwnd);
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to create window", _);
    }
  }
  [[nodiscard]] window(                                                                                //
    const window& Parent, const wclass& Class, stringable auto&& Title, style Style, numeric auto&& X, //
    numeric auto&& Y, numeric auto&& Width, numeric auto&& Height, const source& _ = {}) noexcept {
    try {
      if (!Class) throw std::runtime_error("window class is not registered");
      auto title = cvt<cat1>(Title);
      _hwnd = ::CreateWindowExA(int(nat8(Style) >> 32), Class.name.data(), title.data(), int(nat8(Style) & 0xffffffff), //
                                int(X), int(Y), int(Width), int(Height), Parent, nullptr, main::hinstance, nullptr);
      if (!_hwnd) throw std::runtime_error("CreateWindowEx failed");
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to create child window", _);
    }
  }
  window& operator=(window&& w) noexcept {
    if (_hwnd) {
      if (!::DestroyWindow(_hwnd)) {
        auto ee = long(::GetLastError());
        if (ee == ERROR_INVALID_WINDOW_HANDLE) return *this;
        main::log(logger::error, format_error(ee));
        main::log(logger::error, "failed to destroy window");
      }
    }
    _hwnd = exchange(w._hwnd, nullptr);
    return *this;
  }
  ~window() noexcept {
    if (_hwnd) {
      if (!::DestroyWindow(_hwnd)) {
        auto ee = long(::GetLastError());
        if (ee == ERROR_INVALID_WINDOW_HANDLE) return;
        main::log(logger::error, format_error(ee));
        main::log(logger::error, "failed to destroy window");
      }
    }
  }
  [[nodiscard]] string<cat1> text() const {
    auto len = ::GetWindowTextLengthA(_hwnd);
    if (!len) return {};
    string<cat1> s(len, L'\0');
    ::GetWindowTextA(_hwnd, s.data(), len + 1);
    return s;
  }
  template<character Ct> [[nodiscard]] string<Ct> text() const {
    if constexpr (sizeof(Ct) == 1) {
      auto len = ::GetWindowTextLengthA(_hwnd);
      if (!len) return {};
      string<Ct> s(len, L'\0');
      ::GetWindowTextA(_hwnd, reinterpret_cast<char*>(s.data()), len + 1);
      return s;
    } else if constexpr (sizeof(Ct) == 2) {
      auto len = ::GetWindowTextLengthW(_hwnd);
      if (!len) return {};
      string<Ct> s(len, L'\0');
      ::GetWindowTextW(_hwnd, reinterpret_cast<wchar_t*>(s.data()), len + 1);
      return s;
    } else return cvt<uct4>(text<cat2>());
  }
  void text(stringable auto&& Text) noexcept {
    using Ct = remove_cvref<iter_value<decltype(Text)>>;
    if constexpr (sizeof(Ct) == 1) {
      string<Ct> temp(string_view<Ct>(fwd<decltype(Text)>(Text)));
      ::SetWindowTextA(_hwnd, reinterpret_cast<const char*>(temp.data()));
    } else if constexpr (sizeof(Ct) == 2) {
      string<Ct> temp(string_view<Ct>(fwd<decltype(Text)>(Text)));
      ::SetWindowTextW(_hwnd, reinterpret_cast<const wchar_t*>(temp.data()));
    } else text(cvt<cat2>(fwd<decltype(Text)>(Text)));
  }
  [[nodiscard]] vector<int> position() const noexcept {
    return [this](vector<int> v) { return ::GetWindowRect(_hwnd, reinterpret_cast<RECT*>(&v)), mv(v); }({});
  }
  bool position(numeric auto&& X, numeric auto&& Y) noexcept { //
    return ::SetWindowPos(_hwnd, 0, int(X), int(Y), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
  }
  bool position(numeric auto&& X, numeric auto&& Y, numeric auto&& Width, numeric auto&& Height) noexcept { //
    return ::SetWindowPos(_hwnd, 0, int(X), int(Y), int(Width), int(Height), SWP_NOZORDER);
  }
  [[nodiscard]] vector2<int> size() const noexcept {
    return [this](RECT r) { return ::GetClientRect(_hwnd, &r), vector2<int>(r.right, r.bottom); }({});
  }
  bool ok(const string<cat1>& Text, const string<cat1>& Caption = "OK?") const noexcept { //
    return ::MessageBoxA(_hwnd, Text.c_str(), Caption.c_str(), MB_ICONEXCLAMATION | MB_OK) == IDOK;
  }
  bool ok(const string<cat2>& Text, const string<cat2>& Caption = L"OK?") const noexcept { //
    return ::MessageBoxW(_hwnd, Text.c_str(), Caption.c_str(), MB_ICONEXCLAMATION | MB_OK) == IDOK;
  }
  bool ok(stringable auto&& Text, stringable auto&& Caption) const noexcept   //
    requires different_from<remove_cvref<decltype(Text)>, string<cat1>> &&    //
             different_from<remove_cvref<decltype(Caption)>, string<cat1>> && //
             different_from<remove_cvref<decltype(Text)>, string<cat2>> &&    //
             different_from<remove_cvref<decltype(Caption)>, string<cat2>> {
    return ok(cvt<cat1>(fwd<decltype(Text)>(Text)), cvt<cat1>(fwd<decltype(Caption)>(Caption)));
  }
  bool ok(stringable auto&& Text) const noexcept                           //
    requires different_from<remove_cvref<decltype(Text)>, string<cat1>> && //
             different_from<remove_cvref<decltype(Text)>, string<cat2>> {
    return ok(cvt<cat1>(fwd<decltype(Text)>(Text)));
  }

  [[nodiscard]] bool yes(const string<cat1>& Text, const string<cat1>& Caption = "Yes?") const noexcept { //
    return ::MessageBoxA(_hwnd, Text.c_str(), Caption.c_str(), MB_ICONQUESTION | MB_YESNO) == IDYES;
  }
  [[nodiscard]] bool yes(const string<cat2>& Text, const string<cat2>& Caption = L"Yes?") const noexcept { //
    return ::MessageBoxW(_hwnd, Text.c_str(), Caption.c_str(), MB_ICONQUESTION | MB_YESNO) == IDYES;
  }
  [[nodiscard]] bool yes(stringable auto&& Text, stringable auto&& Caption) const noexcept //
    requires different_from<remove_cvref<decltype(Text)>, string<cat1>> &&                 //
             different_from<remove_cvref<decltype(Caption)>, string<cat1>> &&              //
             different_from<remove_cvref<decltype(Text)>, string<cat2>> &&                 //
             different_from<remove_cvref<decltype(Caption)>, string<cat2>> {
    return yes(cvt<cat1>(fwd<decltype(Text)>(Text)), cvt<cat1>(fwd<decltype(Caption)>(Caption)));
  }
  [[nodiscard]] bool yes(stringable auto&& Text) const noexcept            //
    requires different_from<remove_cvref<decltype(Text)>, string<cat1>> && //
             different_from<remove_cvref<decltype(Text)>, string<cat2>> {
    return yes(cvt<cat1>(fwd<decltype(Text)>(Text)));
  }
  /// waits for a message to be posted to the window
  bool wait_message() noexcept { //
    return ::GetMessageW(reinterpret_cast<MSG*>(&message), _hwnd, 0, 0) > 0;
  }
  /// waits for a message to be posted to the window within a range
  bool wait_message(nat4 Min, nat4 Max) noexcept { //
    return ::GetMessageW(reinterpret_cast<MSG*>(&message), _hwnd, Min, Max) > 0;
  }
  /// peeks for a message to be posted to the window
  bool peek_message(nat4 Remove = 1) noexcept { //
    return ::PeekMessageW(reinterpret_cast<MSG*>(&message), _hwnd, 0, 0, Remove);
  }
  /// peeks for a message to be posted to the window within a range
  bool peek_message(nat4 Min, nat4 Max, nat4 Remove = 1) noexcept { //
    return ::PeekMessageW(reinterpret_cast<MSG*>(&message), _hwnd, Min, Max, Remove);
  }
  /// sends a message to the window
  void send_message(wmessage Message, nat8 WParam = 0, int8 LParam = 0) noexcept { //
    ::SendMessageW(_hwnd, nat4(Message), WParam, LParam);
  }
  /// posts a message to the window
  void post_message(wmessage Message, nat8 WParam = 0, int8 LParam = 0) noexcept { //
    ::PostMessageW(_hwnd, nat4(Message), WParam, LParam);
  }
  /// posts a quit message to the window
  void post_quit_message(nat4 ExitCode = 0) noexcept { ::PostQuitMessage(ExitCode); }
};
static_assert(sizeof(window) == sizeof(HWND) + sizeof(window::message_t) + sizeof(vector2<int>));
using enum window::style;

/** \class yw::wprocedure
 *  \brief defines a window procedure
 *  \note usage: `window_procedure wp{[](window& w, wmessage m, nat8 wp, int8 lp) -> bool { ... }};`
 */

template<invocable_r<bool, window&, wmessage, nat8, int8> Fn> struct window_procedure : public Fn {
  inline static Fn fn{};
  static LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) noexcept {
    list<HWND, vector2<int>, window::message_t> Dummy{hwnd, {}, {hwnd, wmessage(msg), wp, lp}};
    if (fn(reinterpret_cast<window&>(Dummy), wmessage(msg), wp, lp)) return 0;
    else return ::DefWindowProcW(hwnd, msg, wp, lp);
  }
  operator WNDPROC() const noexcept { return wndproc; }
};

} // namespace yw
