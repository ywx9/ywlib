#pragma once

#include "range.hpp"
#include "std.hpp"
#include "xv.hpp"

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

/** \fn bool yw::ok(stringable auto&& Text, stringable auto&& Caption)
 *  \brief displays a message box with an OK button
 *  \param Text text to display
 *  \param Caption caption of the message box (default: "OK?")
 *  \return always `true`
 ******************************************************************************/

inline bool ok(const format_string<cat1> Text, const format_string<cat1> Caption = "OK?") { //
  return ::MessageBoxA(nullptr, Text.get().data(), Caption.get().data(), MB_OK) == IDOK;
}
inline bool ok(const format_string<cat2> Text, const format_string<cat2> Caption = L"OK?") { //
  return ::MessageBoxW(nullptr, Text.get().data(), Caption.get().data(), MB_OK) == IDOK;
}
inline bool ok(stringable auto&& Text, stringable auto&& Caption)        //
  requires (!(convertible_to<decltype(Text), format_string<cat1>> &&     //
              convertible_to<decltype(Caption), format_string<cat1>>) && //
            !(convertible_to<decltype(Text), format_string<cat2>> &&     //
              convertible_to<decltype(Caption), format_string<cat2>>)) {
  if constexpr (same_as<cat1, iter_value<decltype(Text)>, iter_value<decltype(Caption)>>) {
    string<cat1> s1(string_view<cat1>(Text)), s2(string_view<cat1>(Caption));
    return ::MessageBoxA(nullptr, s1.data(), s2.data(), MB_OK) == IDOK;
  } else {
    string<cat2> s1(cvt<cat2>(Text)), s2(cvt<cat2>(Caption));
    return ::MessageBoxW(nullptr, s1.data(), s2.data(), MB_OK) == IDOK;
  }
}
inline bool ok(stringable auto&& Text)                              //
  requires (!convertible_to<decltype(Text), format_string<cat1>> && //
            !convertible_to<decltype(Text), format_string<cat2>>) { //
  if constexpr (same_as<cat1, iter_value<decltype(Text)>>) {
    string<cat1> s(string_view<cat1>(Text));
    return ::MessageBoxA(nullptr, s.data(), "OK?", MB_OK) == IDOK;
  } else {
    string<cat2> s(cvt<cat2>(Text));
    return ::MessageBoxW(nullptr, s.data(), L"OK?", MB_OK) == IDOK;
  }
}

/** \fn bool yw::yes(stringable auto&& Text, stringable auto&& Caption)
 *  \brief displays a message box with Yes and No buttons
 *  \param Text text to display
 *  \param Caption caption of the message box (default: "Yes?")
 *  \return `true` if the user clicked Yes, `false` if the user clicked No
 ******************************************************************************/

inline bool yes(const format_string<cat1> Text, const format_string<cat1> Caption = "Yes?") { //
  return ::MessageBoxA(nullptr, Text.get().data(), Caption.get().data(), MB_YESNO) == IDYES;
}
inline bool yes(const format_string<cat2> Text, const format_string<cat2> Caption = L"Yes?") { //
  return ::MessageBoxW(nullptr, Text.get().data(), Caption.get().data(), MB_YESNO) == IDYES;
}
inline bool yes(stringable auto&& Text, stringable auto&& Caption)       //
  requires (!(convertible_to<decltype(Text), format_string<cat1>> &&     //
              convertible_to<decltype(Caption), format_string<cat1>>) && //
            !(convertible_to<decltype(Text), format_string<cat2>> &&     //
              convertible_to<decltype(Caption), format_string<cat2>>)) {
  if constexpr (same_as<cat1, iter_value<decltype(Text)>, iter_value<decltype(Caption)>>) {
    string<cat1> s1(string_view<cat1>(Text)), s2(string_view<cat1>(Caption));
    return ::MessageBoxA(nullptr, s1.data(), s2.data(), MB_YESNO) == IDYES;
  } else {
    string<cat2> s1(cvt<cat2>(Text)), s2(cvt<cat2>(Caption));
    return ::MessageBoxW(nullptr, s1.data(), s2.data(), MB_YESNO) == IDYES;
  }
}
inline bool yes(stringable auto&& Text)                             //
  requires (!convertible_to<decltype(Text), format_string<cat1>> && //
            !convertible_to<decltype(Text), format_string<cat2>>) { //
  if constexpr (same_as<cat1, iter_value<decltype(Text)>>) {
    string<cat1> s(string_view<cat1>(Text));
    return ::MessageBoxA(nullptr, s.data(), "Yes?", MB_YESNO) == IDYES;
  } else {
    string<cat2> s(cvt<cat2>(Text));
    return ::MessageBoxW(nullptr, s.data(), L"Yes?", MB_YESNO) == IDYES;
  }
}

/** \fn string<cat1> yw::format_error(HRESULT hr)
 *  \brief formats a windows error code
 ******************************************************************************/

inline string<cat1> format_error(same_as<unsigned long> auto hr) {
  string<cat1> buffer(256, {});
  auto n = ::FormatMessageA(0x1200, nullptr, hr, 0, buffer.data(), 255, nullptr);
  buffer.resize(n);
  return buffer;
}

/** \class yw::stopwatch
 *  \brief class to measure time
 ******************************************************************************/

class stopwatch {
  inline static fat8 freq = [](LARGE_INTEGER _) { return ::QueryPerformanceFrequency(&_), fat8(_.QuadPart); }({});
  mutable LARGE_INTEGER li;
  int8 last;
public:
  stopwatch() noexcept : li{}, last{(::QueryPerformanceCounter(&li), li.QuadPart)} {}
  fat8 operator()() const noexcept { return *this; }
  operator fat8() const noexcept { return ::QueryPerformanceCounter(&li), (li.QuadPart - last) / freq; }
  explicit operator fat4() const noexcept { return float(operator fat8()); }
  void reset() noexcept { ::QueryPerformanceCounter(&li), last = li.QuadPart; }
  fat8 split() noexcept { return ::QueryPerformanceCounter(&li), (li.QuadPart - exchange(last, li.QuadPart)) / freq; }
};

/** \namespace yw::main
 *
 */

namespace main {

inline const HINSTANCE hinstance = ::GetModuleHandleW(nullptr);
inline logger log{path("log.log")};
inline const array<string<cat1>> args = [](int argc) noexcept {
  try {
    auto a = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
    array<string<cat1>> r(argc);
    for (; 0 <= --argc;) r[argc] = cvt<cat1>(a[argc]);
    ::LocalFree(a), log.path = path(r[0]).replace_extension(".log");
    return r;
  } catch (const std::exception& e) {
    log(logger::critical, e.what());
    log(logger::critical, "failed to obtain executable arguments");
    return array<string<cat1>>{};
  }
}({});
inline const string<cat1> username = [](DWORD d) noexcept {
  try {
    ::GetUserNameW(nullptr, &d);
    string<cat2> r(d - 1, {});
    ::GetUserNameW(r.data(), &d);
    if (r.empty()) log(logger::critical, "failed to obtain user name");
    return cvt<cat1>(r);
  } catch (const std::exception& e) {
    log(logger::critical, e.what());
    log(logger::critical, "failed to obtain user name");
    return string<cat1>{};
  }
}({});
}

/** \class wicon
 *  \brief windows icon class
 ******************************************************************************/

class wicon {
  wicon(const wicon&) = delete;
  wicon& operator=(const wicon&) = delete;
protected:
  HICON _hicon{};
  wicon(HICON Icon) noexcept : _hicon(Icon) {}
public:
  class predefined;
  wicon(const predefined& PredefinedIcon) = delete;
  wicon() noexcept = default;
  wicon(wicon&& i) noexcept : _hicon(exchange(i._hicon, nullptr)) {}
  operator HICON() const noexcept { return _hicon; }
  explicit operator bool() const noexcept { return _hicon != nullptr; }
  [[nodiscard]] wicon(const path& File, const source& _ = {}) noexcept {
    try {
      auto bitmap = (HBITMAP)::LoadImageW(nullptr, File.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
      if (!bitmap) throw std::runtime_error("failed to load image from file");
      ICONINFO ii{.fIcon = TRUE, .hbmMask = bitmap, .hbmColor = bitmap};
      _hicon = ::CreateIconIndirect(&ii);
      ::DeleteObject(bitmap);
      if (_hicon) throw std::runtime_error("failed to create icon from image");
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to load icon from file", _);
    } catch (...) { main::log(logger::error, "failed to load icon from file", _); }
  }
  wicon& operator=(wicon&& Icon) noexcept {
    if (_hicon && !::DestroyIcon(_hicon)) main::log(logger::error, "failed to destroy icon");
    return _hicon = exchange(Icon._hicon, nullptr), *this;
  }
  ~wicon() noexcept {
    if (_hicon && !::DestroyIcon(_hicon)) main::log(logger::error, "failed to destroy icon");
  }
};

/** \class wicon::predefined
 *  \brief predefined system icons
 ******************************************************************************/

class wicon::predefined : public wicon {
  template<int I> predefined(constant<I>) noexcept //
    : wicon((HICON)::LoadIcon(0, MAKEINTRESOURCE(I))) {}
  predefined(predefined&&) = delete;
  predefined(const predefined&) = delete;
  predefined& operator=(predefined&&) = delete;
  predefined& operator=(const predefined&) = delete;
public:
  static const wicon::predefined app;
  static const wicon::predefined error;
  static const wicon::predefined question;
  static const wicon::predefined warning;
  static const wicon::predefined info;
  static const wicon::predefined shield;
  ~predefined() noexcept { _hicon = nullptr; }
  using wicon::operator HICON;
  using wicon::operator bool;
};

inline const wicon::predefined wicon::predefined::app{constant<32512>{}};
inline const wicon::predefined wicon::predefined::error{constant<32513>{}};
inline const wicon::predefined wicon::predefined::question{constant<32514>{}};
inline const wicon::predefined wicon::predefined::warning{constant<32515>{}};
inline const wicon::predefined wicon::predefined::info{constant<32516>{}};
inline const wicon::predefined wicon::predefined::shield{constant<32518>{}};

inline const wicon::predefined& app_icon = wicon::predefined::app;
inline const wicon::predefined& error_icon = wicon::predefined::error;
inline const wicon::predefined& question_icon = wicon::predefined::question;
inline const wicon::predefined& warning_icon = wicon::predefined::warning;
inline const wicon::predefined& info_icon = wicon::predefined::info;
inline const wicon::predefined& shield_icon = wicon::predefined::shield;

static_assert(sizeof(wicon) == sizeof(HICON));
static_assert(sizeof(wicon::predefined) == sizeof(HICON));

/** \class wcursor
 *  \brief windows cursor class
 ******************************************************************************/

class wcursor {
  wcursor(const wcursor&) = delete;
  wcursor& operator=(const wcursor&) = delete;
protected:
  HCURSOR _hcursor{};
  wcursor(HCURSOR Cursor) noexcept : _hcursor(Cursor) {}
public:
  class predefined;
  wcursor(const predefined& PredefinedCursor) = delete;
  wcursor() noexcept = default;
  wcursor(wcursor&& c) noexcept : _hcursor(exchange(c._hcursor, nullptr)) {}
  operator HCURSOR() const noexcept { return _hcursor; }
  explicit operator bool() const noexcept { return _hcursor != nullptr; }
  [[nodiscard]] wcursor(const path& File, const source& _ = {}) noexcept {
    try {
      _hcursor = ::LoadCursorFromFileW(File.c_str());
      if (_hcursor) throw std::runtime_error("failed to load cursor from file");
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to create cursor from file", _);
    } catch (...) { main::log(logger::error, "failed to create cursor from file", _); }
  }
  wcursor& operator=(wcursor&& Cursor) noexcept {
    if (_hcursor && !::DestroyCursor(_hcursor)) main::log(logger::error, "failed to destroy cursor");
    return _hcursor = exchange(Cursor._hcursor, nullptr), *this;
  }
  ~wcursor() noexcept {
    if (_hcursor && !::DestroyCursor(_hcursor)) main::log(logger::error, "failed to destroy cursor");
  }
};

/** \class wcursor::predefined
 *  \brief predefined system cursors
 ******************************************************************************/

class wcursor::predefined : public wcursor {
  template<int I> predefined(constant<I>) noexcept //
    : wcursor((HCURSOR)::LoadCursor(0, MAKEINTRESOURCE(I))) {}
  predefined(predefined&&) = delete;
  predefined(const predefined&) = delete;
  predefined& operator=(predefined&&) = delete;
  predefined& operator=(const predefined&) = delete;
public:
  static const wcursor::predefined arrow;
  static const wcursor::predefined ibeam;
  static const wcursor::predefined wait;
  static const wcursor::predefined cross;
  static const wcursor::predefined uparrow;
  static const wcursor::predefined sizeall;
  static const wcursor::predefined sizens;
  static const wcursor::predefined sizewe;
  static const wcursor::predefined no;
  static const wcursor::predefined hand;
  static const wcursor::predefined help;
  ~predefined() noexcept { _hcursor = nullptr; }
  using wcursor::operator HCURSOR;
  using wcursor::operator bool;
};

inline const wcursor::predefined wcursor::predefined::arrow{constant<32512>{}};
inline const wcursor::predefined wcursor::predefined::ibeam{constant<32513>{}};
inline const wcursor::predefined wcursor::predefined::wait{constant<32514>{}};
inline const wcursor::predefined wcursor::predefined::cross{constant<32515>{}};
inline const wcursor::predefined wcursor::predefined::uparrow{constant<32516>{}};
inline const wcursor::predefined wcursor::predefined::sizeall{constant<32646>{}};
inline const wcursor::predefined wcursor::predefined::sizens{constant<32645>{}};
inline const wcursor::predefined wcursor::predefined::sizewe{constant<32644>{}};
inline const wcursor::predefined wcursor::predefined::no{constant<32648>{}};
inline const wcursor::predefined wcursor::predefined::hand{constant<32649>{}};
inline const wcursor::predefined wcursor::predefined::help{constant<32651>{}};

inline const wcursor::predefined& arrow_cursor = wcursor::predefined::arrow;
inline const wcursor::predefined& ibeam_cursor = wcursor::predefined::ibeam;
inline const wcursor::predefined& wait_cursor = wcursor::predefined::wait;
inline const wcursor::predefined& cross_cursor = wcursor::predefined::cross;
inline const wcursor::predefined& uparrow_cursor = wcursor::predefined::uparrow;
inline const wcursor::predefined& sizeall_cursor = wcursor::predefined::sizeall;
inline const wcursor::predefined& sizens_cursor = wcursor::predefined::sizens;
inline const wcursor::predefined& sizewe_cursor = wcursor::predefined::sizewe;
inline const wcursor::predefined& hand_cursor = wcursor::predefined::hand;
inline const wcursor::predefined& help_cursor = wcursor::predefined::help;

static_assert(sizeof(wcursor) == sizeof(HCURSOR));
static_assert(sizeof(wcursor::predefined) == sizeof(HCURSOR));

/** \class wclass
 *  \brief wndclass class
 ******************************************************************************/

class wclass {
  wclass(const wclass&) = delete;
  wclass& operator=(const wclass&) = delete;
  HBRUSH to_brush(const color& c) noexcept {
    auto a = _mm_cvtps_epi32(xvmul(c, xv_constant<255>));
    a = _mm_shuffle_epi8(a, xv_constant<0xc0080400>);
    return ::CreateSolidBrush(natcast(_mm_cvtsi128_si32(a)));
  }
protected:
  string_view<cat1> _name{};
  HBRUSH _hbrush{};
  wclass(string_view<cat1> Name) noexcept : _name(Name) {}
public:
  class control;
  wclass(const control& PredefinedClass) = delete;
  enum class style : nat4 {
    cs_vredraw = 0x0001,
    cs_hredraw = 0x0002,
    cs_dblclks = 0x0008,
    cs_owndc = 0x0020,
    cs_classdc = 0x0040,
    cs_parentdc = 0x0080,
    cs_noclose = 0x0200,
    cs_savebits = 0x0800,
    cs_bytealignclient = 0x1000,
    cs_bytealignwindow = 0x2000,
    cs_globalclass = 0x4000,
    cs_ime = 0x10000,
    cs_dropshadow = 0x20000,
  };
  friend constexpr style operator~(style a) noexcept { return style(~nat4(a)); }
  friend constexpr style operator|(style a, style b) noexcept { return style(nat4(a) | nat4(b)); }
  friend constexpr style operator&(style a, style b) noexcept { return style(nat4(a) & nat4(b)); }
  friend constexpr style operator^(style a, style b) noexcept { return style(nat4(a) ^ nat4(b)); }
  const string_view<cat1>& name = _name;
  wclass() noexcept = default;
  wclass(wclass&& wc) noexcept : _name(exchange(wc._name, {})), _hbrush(exchange(wc._hbrush, nullptr)) {}
  operator string_view<cat1>() const noexcept { return _name; }
  explicit operator bool() const noexcept { return !_name.empty(); }
  template<typename Ct> explicit operator string<Ct>() const noexcept { return cvt<Ct>(_name); }
  [[nodiscard]] wclass(                                                                 //
    format_string<cat1> Name, WNDPROC WndProc, wclass::style Style,                     //
    const color& Background = color::white, const wicon& Icon = wicon::predefined::app, //
    const wcursor& Cursor = wcursor::predefined::arrow, const source& _ = {}) noexcept
    : _name(Name.get()), _hbrush(to_brush(Background)) {
    try {
      if (!_hbrush) throw std::runtime_error("failed to create brush");
      auto name_ = cvt<cat2>(_name);
      WNDCLASSEXW wc{sizeof(WNDCLASSEXW), nat4(Style), WndProc};
      wc.hInstance = main::hinstance, wc.hIcon = Icon, wc.hCursor = Cursor;
      wc.hbrBackground = _hbrush, wc.lpszClassName = name_.c_str();
      if (!::RegisterClassExW(&wc)) main::log(logger::error, "failed to register window class", _);
    } catch (const std::exception& e) {
      _name = {}, _hbrush = nullptr;
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to register window class", _);
    } catch (...) { _name = {}, _hbrush = nullptr, main::log(logger::error, "failed to register window class", _); }
  }
  wclass& operator=(wclass&& wc) noexcept {
    if (_hbrush && !::DeleteObject(_hbrush)) main::log(logger::error, "failed to destroy brush");
    if (auto name_ = cvt<cat2>(_name); !name_.empty() && !::UnregisterClassW(name_.c_str(), main::hinstance)) //
      main::log(logger::error, "failed to unregister window class");
    return _name = exchange(wc._name, {}), _hbrush = exchange(wc._hbrush, nullptr), *this;
  }
  ~wclass() noexcept {
    if (_hbrush && !::DeleteObject(_hbrush)) main::log(logger::error, "failed to destroy brush");
    if (auto name_ = cvt<cat2>(_name); !name.empty() && !::UnregisterClassW(name_.c_str(), main::hinstance)) //
      main::log(logger::error, "failed to unregister window class");
  }
};

/** \class wclass::control
 *  \brief predefined window controls
 ******************************************************************************/

class wclass::control : public wclass {
  control(format_string<cat1> Name) noexcept : wclass(Name.get()) {}
  control(control&&) = delete;
  control(const control&) = delete;
  control& operator=(control&&) = delete;
  control& operator=(const control&) = delete;
public:
  ~control() noexcept { _name = {}, _hbrush = nullptr; }
  static const wclass::control button;
  static const wclass::control edit;
  static const wclass::control static_;
  static const wclass::control listbox;
  static const wclass::control scrollbar;
  static const wclass::control combobox;
};

inline const wclass::control wclass::control::button{"BUTTON"};
inline const wclass::control wclass::control::edit{"EDIT"};
inline const wclass::control wclass::control::static_{"STATIC"};
inline const wclass::control wclass::control::listbox{"LISTBOX"};
inline const wclass::control wclass::control::scrollbar{"SCROLLBAR"};
inline const wclass::control wclass::control::combobox{"COMBOBOX"};

inline const wclass::control& button_control = wclass::control::button;
inline const wclass::control& edit_control = wclass::control::edit;
inline const wclass::control& static_control = wclass::control::static_;
inline const wclass::control& listbox_control = wclass::control::listbox;
inline const wclass::control& scrollbar_control = wclass::control::scrollbar;
inline const wclass::control& combobox_control = wclass::control::combobox;

using enum wclass::style;

/** \enum wmessage
 *  \brief windows messages
 ******************************************************************************/

enum class wmessage : yw::nat4 {
  wm_null = 0x0000,
  wm_create = 0x0001,
  wm_destroy = 0x0002,
  wm_move = 0x0003,
  wm_size = 0x0005,
  wm_activate = 0x0006,
  wm_setfocus = 0x0007,
  wm_killfocus = 0x0008,
  wm_enable = 0x000A,
  wm_setredraw = 0x000B,
  wm_settext = 0x000C,
  wm_gettext = 0x000D,
  wm_gettextlength = 0x000E,
  wm_paint = 0x000F,
  wm_close = 0x0010,
  wm_queryendsession = 0x0011,
  wm_queryopen = 0x0013,
  wm_endsession = 0x0016,
  wm_quit = 0x0012,
  wm_erasebkgnd = 0x0014,
  wm_syscolorchange = 0x0015,
  wm_showwindow = 0x0018,
  wm_wininichange = 0x001A,
  wm_settingchange = wm_wininichange,
  wm_devmodechange = 0x001B,
  wm_activateapp = 0x001C,
  wm_fontchange = 0x001D,
  wm_timechange = 0x001E,
  wm_cancelmode = 0x001F,
  wm_setcursor = 0x0020,
  wm_mouseactivate = 0x0021,
  wm_childactivate = 0x0022,
  wm_queuesync = 0x0023,
  wm_getminmaxinfo = 0x0024,
  wm_painticon = 0x0026,
  wm_iconerasebkgnd = 0x0027,
  wm_nextdlgctl = 0x0028,
  wm_spoolerstatus = 0x002A,
  wm_drawitem = 0x002B,
  wm_measureitem = 0x002C,
  wm_deleteitem = 0x002D,
  wm_vkeytoitem = 0x002E,
  wm_chartoitem = 0x002F,
  wm_setfont = 0x0030,
  wm_getfont = 0x0031,
  wm_sethotkey = 0x0032,
  wm_gethotkey = 0x0033,
  wm_querydragicon = 0x0037,
  wm_compareitem = 0x0039,
  wm_getobject = 0x003D,
  wm_compacting = 0x0041,
  wm_commnotify = 0x0044,
  wm_windowposchanging = 0x0046,
  wm_windowposchanged = 0x0047,
  wm_power = 0x0048,
  wm_copydata = 0x004A,
  wm_canceljournal = 0x004B,
  wm_notify = 0x004E,
  wm_inputlangchangerequest = 0x0050,
  wm_inputlangchange = 0x0051,
  wm_tcard = 0x0052,
  wm_help = 0x0053,
  wm_userchanged = 0x0054,
  wm_notifyformat = 0x0055,
  wm_contextmenu = 0x007B,
  wm_stylechanging = 0x007C,
  wm_stylechanged = 0x007D,
  wm_displaychange = 0x007E,
  wm_geticon = 0x007F,
  wm_seticon = 0x0080,
  wm_nccreate = 0x0081,
  wm_ncdestroy = 0x0082,
  wm_nccalcsize = 0x0083,
  wm_nchittest = 0x0084,
  wm_ncpaint = 0x0085,
  wm_ncactivate = 0x0086,
  wm_getdlgcode = 0x0087,
  wm_syncpaint = 0x0088,
  wm_ncmousemove = 0x00A0,
  wm_nclbuttondown = 0x00A1,
  wm_nclbuttonup = 0x00A2,
  wm_nclbuttondblclk = 0x00A3,
  wm_ncrbuttondown = 0x00A4,
  wm_ncrbuttonup = 0x00A5,
  wm_ncrbuttondblclk = 0x00A6,
  wm_ncmbuttondown = 0x00A7,
  wm_ncmbuttonup = 0x00A8,
  wm_ncmbuttondblclk = 0x00A9,
  wm_ncxbuttondown = 0x00AB,
  wm_ncxbuttonup = 0x00AC,
  wm_ncxbuttondblclk = 0x00AD,
  wm_input_device_change = 0x00FE,
  wm_input = 0x00FF,
  wm_keyfirst = 0x0100,
  wm_keydown = 0x0100,
  wm_keyup = 0x0101,
  wm_char = 0x0102,
  wm_deadchar = 0x0103,
  wm_syskeydown = 0x0104,
  wm_syskeyup = 0x0105,
  wm_syschar = 0x0106,
  wm_sysdeadchar = 0x0107,
  wm_unichar = 0x0109,
  wm_keylast = 0x0109,
  wm_ime_startcomposition = 0x010D,
  wm_ime_endcomposition = 0x010E,
  wm_ime_composition = 0x010F,
  wm_ime_keylast = 0x010F,
  wm_initdialog = 0x0110,
  wm_command = 0x0111,
  wm_syscommand = 0x0112,
  wm_timer = 0x0113,
  wm_hscroll = 0x0114,
  wm_vscroll = 0x0115,
  wm_initmenu = 0x0116,
  wm_initmenupopup = 0x0117,
  wm_gesture = 0x0119,
  wm_gesturenotify = 0x011A,
  wm_menuselect = 0x011F,
  wm_menuchar = 0x0120,
  wm_enteridle = 0x0121,
  wm_menurbuttonup = 0x0122,
  wm_menudrag = 0x0123,
  wm_menugetobject = 0x0124,
  wm_uninitmenupopup = 0x0125,
  wm_menucommand = 0x0126,
  wm_changeuistate = 0x0127,
  wm_updateuistate = 0x0128,
  wm_queryuistate = 0x0129,
  wm_ctlcolormsgbox = 0x0132,
  wm_ctlcoloredit = 0x0133,
  wm_ctlcolorlistbox = 0x0134,
  wm_ctlcolorbtn = 0x0135,
  wm_ctlcolordlg = 0x0136,
  wm_ctlcolorscrollbar = 0x0137,
  wm_ctlcolorstatic = 0x0138,
  mn_gethmenu = 0x01E1,
  wm_mousefirst = 0x0200,
  wm_mousemove = 0x0200,
  wm_lbuttondown = 0x0201,
  wm_lbuttonup = 0x0202,
  wm_lbuttondblclk = 0x0203,
  wm_rbuttondown = 0x0204,
  wm_rbuttonup = 0x0205,
  wm_rbuttondblclk = 0x0206,
  wm_mbuttondown = 0x0207,
  wm_mbuttonup = 0x0208,
  wm_mbuttondblclk = 0x0209,
  wm_mousewheel = 0x020A,
  wm_xbuttondown = 0x020B,
  wm_xbuttonup = 0x020C,
  wm_xbuttondblclk = 0x020D,
  wm_mousehwheel = 0x020E,
  wm_mouselast = 0x020E,
  wm_parentnotify = 0x0210,
  wm_entermenuloop = 0x0211,
  wm_exitmenuloop = 0x0212,
  wm_nextmenu = 0x0213,
  wm_sizing = 0x0214,
  wm_capturechanged = 0x0215,
  wm_moving = 0x0216,
  wm_powerbroadcast = 0x0218,
  wm_devicechange = 0x0219,
  wm_mdicreate = 0x0220,
  wm_mdidestroy = 0x0221,
  wm_mdiactivate = 0x0222,
  wm_mdirestore = 0x0223,
  wm_mdinext = 0x0224,
  wm_mdimaximize = 0x0225,
  wm_mditile = 0x0226,
  wm_mdicascade = 0x0227,
  wm_mdiiconarrange = 0x0228,
  wm_mdigetactive = 0x0229,
  wm_mdisetmenu = 0x0230,
  wm_entersizemove = 0x0231,
  wm_exitsizemove = 0x0232,
  wm_dropfiles = 0x0233,
  wm_mdirefreshmenu = 0x0234,
  wm_pointerdevicechange = 0x238,
  wm_pointerdeviceinrange = 0x239,
  wm_pointerdeviceoutofrange = 0x23A,
  wm_touch = 0x0240,
  wm_ncpointerupdate = 0x0241,
  wm_ncpointerdown = 0x0242,
  wm_ncpointerup = 0x0243,
  wm_pointerupdate = 0x0245,
  wm_pointerdown = 0x0246,
  wm_pointerup = 0x0247,
  wm_pointerenter = 0x0249,
  wm_pointerleave = 0x024A,
  wm_pointeractivate = 0x024B,
  wm_pointercapturechanged = 0x024C,
  wm_touchhittesting = 0x024D,
  wm_pointerwheel = 0x024E,
  wm_pointerhwheel = 0x024F,
  dm_pointerhittest = 0x0250,
  wm_pointerroutedto = 0x0251,
  wm_pointerroutedaway = 0x0252,
  wm_pointerroutedreleased = 0x0253,
  wm_ime_setcontext = 0x0281,
  wm_ime_notify = 0x0282,
  wm_ime_control = 0x0283,
  wm_ime_compositionfull = 0x0284,
  wm_ime_select = 0x0285,
  wm_ime_char = 0x0286,
  wm_ime_request = 0x0288,
  wm_ime_keydown = 0x0290,
  wm_ime_keyup = 0x0291,
  wm_mousehover = 0x02A1,
  wm_mouseleave = 0x02A3,
  wm_ncmousehover = 0x02A0,
  wm_ncmouseleave = 0x02A2,
  wm_wtssession_change = 0x02B1,
  wm_tablet_first = 0x02c0,
  wm_tablet_last = 0x02df,
  wm_dpichanged = 0x02E0,
  wm_dpichanged_beforeparent = 0x02E2,
  wm_dpichanged_afterparent = 0x02E3,
  wm_getdpiscaledsize = 0x02E4,
  wm_cut = 0x0300,
  wm_copy = 0x0301,
  wm_paste = 0x0302,
  wm_clear = 0x0303,
  wm_undo = 0x0304,
  wm_renderformat = 0x0305,
  wm_renderallformats = 0x0306,
  wm_destroyclipboard = 0x0307,
  wm_drawclipboard = 0x0308,
  wm_paintclipboard = 0x0309,
  wm_vscrollclipboard = 0x030A,
  wm_sizeclipboard = 0x030B,
  wm_askcbformatname = 0x030C,
  wm_changecbchain = 0x030D,
  wm_hscrollclipboard = 0x030E,
  wm_querynewpalette = 0x030F,
  wm_paletteischanging = 0x0310,
  wm_palettechanged = 0x0311,
  wm_hotkey = 0x0312,
  wm_print = 0x0317,
  wm_printclient = 0x0318,
  wm_appcommand = 0x0319,
  wm_themechanged = 0x031A,
  wm_clipboardupdate = 0x031D,
  wm_dwmcompositionchanged = 0x031E,
  wm_dwmncrenderingchanged = 0x031F,
  wm_dwmcolorizationcolorchanged = 0x0320,
  wm_dwmwindowmaximizedchange = 0x0321,
  wm_dwmsendiconicthumbnail = 0x0323,
  wm_dwmsendiconiclivepreviewbitmap = 0x0326,
  wm_gettitlebarinfoex = 0x033F,
  wm_handheldfirst = 0x0358,
  wm_handheldlast = 0x035F,
  wm_affirst = 0x0360,
  wm_aflast = 0x037F,
  wm_penwinfirst = 0x0380,
  wm_penwinlast = 0x038F,
  wm_app = 0x8000,
  wm_user = 0x0400,
};

using enum wmessage;
constexpr bool operator==(wmessage a, nat b) noexcept { return nat(a) == b; }
constexpr bool operator==(nat a, wmessage b) noexcept { return a == nat(b); }
constexpr auto operator<=>(wmessage a, nat b) noexcept { return nat(a) <=> b; }
constexpr auto operator<=>(nat a, wmessage b) noexcept { return a <=> nat(b); }

/** \enum virtual_key
 *  \brief virtual key codes
 ******************************************************************************/

enum class virtual_key : nat4 {
  vk_lbutton = 0x01,
  vk_rbutton = 0x02,
  vk_cancel = 0x03,
  vk_mbutton = 0x04,
  vk_xbutton1 = 0x05,
  vk_xbutton2 = 0x06,
  vk_back = 0x08,
  vk_tab = 0x09,
  vk_clear = 0x0C,
  vk_return = 0x0D,
  vk_shift = 0x10,
  vk_control = 0x11,
  vk_menu = 0x12,
  vk_pause = 0x13,
  vk_capital = 0x14,
  vk_kana = 0x15,
  vk_hangul = 0x15,
  vk_junja = 0x17,
  vk_final = 0x18,
  vk_hanja = 0x19,
  vk_kanji = 0x19,
  vk_escape = 0x1B,
  vk_convert = 0x1C,
  vk_nonconvert = 0x1D,
  vk_accept = 0x1E,
  vk_modechange = 0x1F,
  vk_space = 0x20,
  vk_prior = 0x21,
  vk_next = 0x22,
  vk_end = 0x23,
  vk_home = 0x24,
  vk_left = 0x25,
  vk_up = 0x26,
  vk_right = 0x27,
  vk_down = 0x28,
  vk_select = 0x29,
  vk_print = 0x2A,
  vk_execute = 0x2B,
  vk_snapshot = 0x2C,
  vk_insert = 0x2D,
  vk_delete = 0x2E,
  vk_help = 0x2F,
  vk_0 = 0x30,
  vk_1 = 0x31,
  vk_2 = 0x32,
  vk_3 = 0x33,
  vk_4 = 0x34,
  vk_5 = 0x35,
  vk_6 = 0x36,
  vk_7 = 0x37,
  vk_8 = 0x38,
  vk_9 = 0x39,
  vk_a = 0x41,
  vk_b = 0x42,
  vk_c = 0x43,
  vk_d = 0x44,
  vk_e = 0x45,
  vk_f = 0x46,
  vk_g = 0x47,
  vk_h = 0x48,
  vk_i = 0x49,
  vk_j = 0x4A,
  vk_k = 0x4B,
  vk_l = 0x4C,
  vk_m = 0x4D,
  vk_n = 0x4E,
  vk_o = 0x4F,
  vk_p = 0x50,
  vk_q = 0x51,
  vk_r = 0x52,
  vk_s = 0x53,
  vk_t = 0x54,
  vk_u = 0x55,
  vk_v = 0x56,
  vk_w = 0x57,
  vk_x = 0x58,
  vk_y = 0x59,
  vk_z = 0x5A,
  vk_lwin = 0x5B,
  vk_rwin = 0x5C,
  vk_apps = 0x5D,
  vk_sleep = 0x5F,
  vk_numpad0 = 0x60,
  vk_numpad1 = 0x61,
  vk_numpad2 = 0x62,
  vk_numpad3 = 0x63,
  vk_numpad4 = 0x64,
  vk_numpad5 = 0x65,
  vk_numpad6 = 0x66,
  vk_numpad7 = 0x67,
  vk_numpad8 = 0x68,
  vk_numpad9 = 0x69,
  vk_multiply = 0x6A,
  vk_add = 0x6B,
  vk_separator = 0x6C,
  vk_subtract = 0x6D,
  vk_decimal = 0x6E,
  vk_divide = 0x6F,
  vk_f1 = 0x70,
  vk_f2 = 0x71,
  vk_f3 = 0x72,
  vk_f4 = 0x73,
  vk_f5 = 0x74,
  vk_f6 = 0x75,
  vk_f7 = 0x76,
  vk_f8 = 0x77,
  vk_f9 = 0x78,
  vk_f10 = 0x79,
  vk_f11 = 0x7A,
  vk_f12 = 0x7B,
  vk_f13 = 0x7C,
  vk_f14 = 0x7D,
  vk_f15 = 0x7E,
  vk_f16 = 0x7F,
  vk_f17 = 0x80,
  vk_f18 = 0x81,
  vk_f19 = 0x82,
  vk_f20 = 0x83,
  vk_f21 = 0x84,
  vk_f22 = 0x85,
  vk_f23 = 0x86,
  vk_f24 = 0x87,
  vk_numlock = 0x90,
  vk_scroll = 0x91,
  vk_oem_nec_equal = 0x92,
  vk_oem_fj_jisho = 0x92,
  vk_oem_fj_masshou = 0x93,
  vk_oem_fj_touroku = 0x94,
  vk_oem_fj_loya = 0x95,
  vk_oem_fj_roya = 0x96,
  vk_lshift = 0xA0,
  vk_rshift = 0xA1,
  vk_lcontrol = 0xA2,
  vk_rcontrol = 0xA3,
  vk_lmenu = 0xA4,
  vk_rmenu = 0xA5,
  vk_browser_back = 0xA6,
  vk_browser_forward = 0xA7,
  vk_browser_refresh = 0xA8,
  vk_browser_stop = 0xA9,
  vk_browser_search = 0xAA,
  vk_browser_favorites = 0xAB,
  vk_browser_home = 0xAC,
  vk_volume_mute = 0xAD,
  vk_volume_down = 0xAE,
  vk_volume_up = 0xAF,
  vk_media_next_track = 0xB0,
  vk_media_prev_track = 0xB1,
  vk_media_stop = 0xB2,
  vk_media_play_pause = 0xB3,
  vk_launch_mail = 0xB4,
  vk_launch_media_select = 0xB5,
  vk_launch_app1 = 0xB6,
  vk_launch_app2 = 0xB7,
  vk_oem_1 = 0xBA,
  vk_oem_plus = 0xBB,
  vk_oem_comma = 0xBC,
  vk_oem_minus = 0xBD,
  vk_oem_period = 0xBE,
  vk_oem_2 = 0xBF,
  vk_oem_3 = 0xC0,
  vk_oem_4 = 0xDB,
  vk_oem_5 = 0xDC,
  vk_oem_6 = 0xDD,
  vk_oem_7 = 0xDE,
  vk_oem_8 = 0xDF,
  vk_oem_ax = 0xE1,
  vk_oem_102 = 0xE2,
  vk_icong = 0xE3,
  vk_icoff = 0xE4,
  vk_attn = 0xF6,
  vk_crsel = 0xF7,
  vk_exsel = 0xF8,
  vk_ereof = 0xF9,
  vk_play = 0xFA,
  vk_zoom = 0xFB,
  vk_noname = 0xFC,
  vk_pa1 = 0xFD,
  vk_oem_clear = 0xFE,
};

using enum virtual_key;
constexpr bool operator==(virtual_key a, nat b) noexcept { return nat(a) == b; }
constexpr bool operator==(nat a, virtual_key b) noexcept { return a == nat(b); }
constexpr auto operator<=>(virtual_key a, nat b) noexcept { return nat(a) <=> b; }
constexpr auto operator<=>(nat a, virtual_key b) noexcept { return a <=> nat(b); }

/** \class yw::window
 *  \brief class to create and manage windows
 ******************************************************************************/

class window {
protected:
  HWND _hwnd{};
  MSG _msg{};
  vector2<int4> _pad{};
  window(HWND Hwnd) noexcept : _hwnd(Hwnd) {}
public:
  enum class style : nat8 {
    ws_maximizebox = 0x10000,
    ws_minimizebox = 0x20000,
    ws_tabstop = 0x10000,
    ws_group = 0x20000,
    ws_thickframe = 0x40000,
    ws_sizebox = ws_thickframe,
    ws_sysmenu = 0x80000,
    ws_hscroll = 0x100000,
    ws_vscroll = 0x200000,
    ws_dlgframe = 0x400000,
    ws_border = 0x800000,
    ws_caption = ws_border | ws_dlgframe,
    ws_maximize = 0x1000000,
    ws_clipchildren = 0x2000000,
    ws_clipsiblings = 0x4000000,
    ws_disabled = 0x8000000,
    ws_visible = 0x10000000,
    ws_minimize = 0x20000000,
    ws_child = 0x40000000,
    ws_popup = 0x80000000,
    ws_overlapped = 0x00000000,
    ws_tiled = ws_overlapped,
    ws_iconic = ws_minimize,
    ws_childwindow = ws_child,
    ws_overlappedwindow = ws_overlapped | ws_caption | ws_sysmenu | ws_thickframe | ws_minimizebox | ws_maximizebox,
    ws_popupwindow = ws_popup | ws_border | ws_sysmenu,
    ws_tiledwindow = ws_overlappedwindow,
    ws_ex_dlgsmodalframe = 0x00000001ULL << 32,
    ws_ex_noparentnotify = 0x00000004ULL << 32,
    ws_ex_topmost = 0x00000008ULL << 32,
    ws_ex_acceptfiles = 0x00000010ULL << 32,
    ws_ex_transparent = 0x00000020ULL << 32,
    ws_ex_mdichild = 0x00000040ULL << 32,
    ws_ex_toolwindow = 0x00000080ULL << 32,
    ws_ex_windowedge = 0x00000100ULL << 32,
    ws_ex_clientedge = 0x00000200ULL << 32,
    ws_ex_contexthelp = 0x00000400ULL << 32,
    ws_ex_right = 0x00001000ULL << 32,
    ws_ex_left = 0x00000000ULL << 32,
    ws_ex_rtlreading = 0x00002000ULL << 32,
    ws_ex_ltrreading = 0x00000000ULL << 32,
    ws_ex_leftscrollbar = 0x00004000ULL << 32,
    ws_ex_rightscrollbar = 0x00000000ULL << 32,
    ws_ex_controlparent = 0x00010000ULL << 32,
    ws_ex_staticedge = 0x00020000ULL << 32,
    ws_ex_appwindow = 0x00040000ULL << 32,
    ws_ex_overlappedwindow = ws_ex_windowedge | ws_ex_clientedge,
    ws_ex_palettewindow = ws_ex_windowedge | ws_ex_toolwindow | ws_ex_topmost,
    ws_ex_layered = 0x00080000ULL << 32,
    ws_ex_noinheritlayout = 0x00100000ULL << 32,
    ws_ex_noredirectionbitmap = 0x00200000ULL << 32,
    ws_ex_layoutrtl = 0x00400000ULL << 32,
    ws_ex_composited = 0x02000000ULL << 32,
    ws_ex_noactivate = 0x08000000ULL << 32,
    es_left = 0x0000,
    es_center = 0x0001,
    es_right = 0x0002,
    es_multiline = 0x0004,
    es_uppercase = 0x0008,
    es_lowercase = 0x0010,
    es_password = 0x0020,
    es_autovscroll = 0x0040,
    es_autohscroll = 0x0080,
    es_nohidesel = 0x0100,
    es_oemconvert = 0x0400,
    es_readonly = 0x0800,
    es_wantreturn = 0x1000,
    es_number = 0x2000,
    bs_pushbutton = 0x00000000,
    bs_defpushbutton = 0x00000001,
    bs_checkbox = 0x00000002,
    bs_autocheckbox = 0x00000003,
    bs_radiobutton = 0x00000004,
    bs_3state = 0x00000005,
    bs_auto3state = 0x00000006,
    bs_groupbox = 0x00000007,
    bs_userbutton = 0x00000008,
    bs_autoradiobutton = 0x00000009,
    bs_pushbox = 0x0000000A,
    bs_ownerdraw = 0x0000000B,
    bs_typemask = 0x0000000F,
    bs_lefttext = 0x00000020,
    bs_text = 0x00000000,
    bs_icon = 0x00000040,
    bs_bitmap = 0x00000080,
    bs_left = 0x00000100,
    bs_right = 0x00000200,
    bs_center = 0x00000300,
    bs_top = 0x00000400,
    bs_bottom = 0x00000800,
    bs_vcenter = 0x00000C00,
    bs_pushlike = 0x00001000,
    bs_multiline = 0x00002000,
    bs_notify = 0x00004000,
    bs_flat = 0x00008000,
    bs_rightbutton = bs_lefttext,
    ss_left = 0x00000000,
    ss_center = 0x00000001,
    ss_right = 0x00000002,
    ss_icon = 0x00000003,
    ss_blackrect = 0x00000004,
    ss_grayrect = 0x00000005,
    ss_whiterect = 0x00000006,
    ss_blackframe = 0x00000007,
    ss_grayframe = 0x00000008,
    ss_whiteframe = 0x00000009,
    ss_useritem = 0x0000000A,
    ss_simple = 0x0000000B,
    ss_leftnowrap = 0x0000000C,
    ss_ownerdraw = 0x0000000D,
    ss_bitmap = 0x0000000E,
    ss_enhmetafile = 0x0000000F,
    ss_etchedhorz = 0x00000010,
    ss_etchedvert = 0x00000011,
    ss_etchedframe = 0x00000012,
    ss_typemask = 0x0000001F,
    ss_realsizecontrol = 0x00000040,
    ss_noprefix = 0x00000080,
    ss_notify = 0x00000100,
    ss_centerimage = 0x00000200,
    ss_rightjust = 0x00000400,
    ss_realsizeimage = 0x00000800,
    ss_sunken = 0x00001000,
    ss_editcontrol = 0x00002000,
    ss_endellipsis = 0x00004000,
    ss_patellipsis = 0x00008000,
    ss_wordellipsis = 0x0000C000,
    ss_ellipsis = 0x0000C000,
    lbs_notify = 0x0001,
    lbs_sort = 0x0002,
    lbs_noredraw = 0x0004,
    lbs_multiplesel = 0x0008,
    lbs_ownerdrawfixed = 0x0010,
    lbs_ownerdrawvariable = 0x0020,
    lbs_hasstrings = 0x0040,
    lbs_usetabstops = 0x0080,
    lbs_nointegralheight = 0x0100,
    lbs_multicolumn = 0x0200,
    lbs_wantkeyboardinput = 0x0400,
    lbs_extendedsel = 0x0800,
    lbs_disable = 0x1000,
    lbs_nodata = 0x2000,
    lbs_nosel = 0x4000,
    lbs_standard = (lbs_notify | lbs_sort | ws_vscroll | ws_border),
    cbs_simple = 0x0001,
    cbs_dropdown = 0x0002,
    cbs_dropdownlist = 0x0003,
    cbs_ownerdrawfixed = 0x0010,
    cbs_ownerdrawvariable = 0x0020,
    cbs_autohscroll = 0x0040,
    cbs_oemconvert = 0x0080,
    cbs_sort = 0x0100,
    cbs_hasstrings = 0x0200,
    cbs_nointegralheight = 0x0400,
    cbs_disablenoscroll = 0x0800,
    cbs_uppercase = 0x2000,
    cbs_lowercase = 0x4000,
    sbs_horz = 0x0000,
    sbs_vert = 0x0001,
    sbs_topalign = 0x0002,
    sbs_leftalign = 0x0002,
    sbs_bottomalign = 0x0004,
    sbs_rightalign = 0x0004,
    sbs_sizeboxtopleftalign = 0x0002,
    sbs_sizeboxbottomrightalign = 0x0004,
    sbs_sizebox = 0x0008,
    sbs_sizegrip = 0x0010,
  };
  friend constexpr style operator~(style a) noexcept { return style(~nat8(a)); }
  friend constexpr style operator|(style a, style b) noexcept { return style(nat8(a) | nat8(b)); }
  friend constexpr style operator&(style a, style b) noexcept { return style(nat8(a) & nat8(b)); }
  friend constexpr style operator^(style a, style b) noexcept { return style(nat8(a) ^ nat8(b)); }
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
  message_t& message = reinterpret_cast<message_t&>(_msg);
  window() noexcept = default;
  window(window&& w) noexcept : _hwnd(exchange(w._hwnd, nullptr)), _msg(w._msg), _pad(w._pad) {}
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
        auto ee = ::GetLastError();
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
        auto ee = ::GetLastError();
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
  bool wait_message() noexcept { return ::GetMessageW(&_msg, _hwnd, 0, 0) > 0; }
  bool wait_message(nat4 Min, nat4 Max) noexcept { return ::GetMessageW(&_msg, _hwnd, Min, Max) > 0; }
  bool peek_message(nat4 Remove = 1) noexcept { return ::PeekMessageW(&_msg, _hwnd, 0, 0, Remove); }
  bool peek_message(nat4 Min, nat4 Max, nat4 Remove = 1) noexcept { return ::PeekMessageW(&_msg, _hwnd, Min, Max, Remove); }
  void send_message(wmessage Message, nat8 WParam = 0, int8 LParam = 0) noexcept { ::SendMessageW(_hwnd, nat4(Message), WParam, LParam); }
  void post_message(wmessage Message, nat8 WParam = 0, int8 LParam = 0) noexcept { ::PostMessageW(_hwnd, nat4(Message), WParam, LParam); }
  void post_quit_message(nat4 ExitCode = 0) noexcept { ::PostQuitMessage(ExitCode); }
};

using enum window::style;

using hwnd = HWND;
inline auto* default_window_proc = ::DefWindowProcW;

}
