#pragma once

#include "2.hpp"

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

/// formats a Windows error message
inline string<cat1> format_windows_error(same_as<long> auto hr) {
  std::string buf(256, '\0');
  buf.resize(::FormatMessageA(0x1200, 0, hr, 0, buf.data(), buf.size(), 0));
  return buf;
}

/// displays a message box with an OK button; always returns `true`
inline bool ok(stringable auto&& Text, stringable auto&& Caption) {
  auto t = cvt<cat2>(Text), c = cvt<cat2>(Caption);
  return ::MessageBoxW(nullptr, t.data(), c.data(), MB_OK) == IDOK;
}

/// displays a message box with an OK button; always returns `true`
inline bool ok(stringable auto&& Text) {
  auto t = cvt<cat2>(Text);
  return ::MessageBoxW(nullptr, t.data(), L"OK?", MB_OK) == IDOK;
}

/// displays a message box with Yes and No buttons; returns `true` for Yes, `false` for No
inline bool yes(stringable auto&& Text, stringable auto&& Caption) {
  auto t = cvt<cat2>(Text), c = cvt<cat2>(Caption);
  return ::MessageBoxW(nullptr, t.data(), c.data(), MB_YESNO) == IDYES;
}

/// displays a message box with Yes and No buttons; returns `true` for Yes, `false` for No
inline bool yes(stringable auto&& Text) {
  auto t = cvt<cat2>(Text);
  return ::MessageBoxW(nullptr, t.data(), L"Yes?", MB_YESNO) == IDYES;
}

/// class to measure time
class stopwatch {
  inline static fat8 f = [](int8 i) { return ::QueryPerformanceFrequency((LARGE_INTEGER*)&i), fat8(i); }({});
  int8 i{};
public:
  /// constructs a stopwatch
  stopwatch() { ::QueryPerformanceCounter((LARGE_INTEGER*)&i); }

  /// returns elapsed time in seconds
  fat8 operator()() const {
    return [this](int8 j) { return ::QueryPerformanceCounter((LARGE_INTEGER*)&j), (j - i) / f; }({});
  }

  operator fat8() const { return operator()(); }
  explicit operator fat4() const { return static_cast<fat4>(operator()()); }

  /// resets the stopwatch
  void reset() { ::QueryPerformanceCounter((LARGE_INTEGER*)&i); }

  /// returns elapsed time in seconds and resets the stopwatch
  fat8 split() {
    return [this](int8 j) { return ::QueryPerformanceCounter((LARGE_INTEGER*)&i), (i - j) / f; }(i);
  }
};

} // namespace yw

export namespace yw::main {

/// instance handle
inline const nat8 hinstance = reinterpret_cast<nat8>(::GetModuleHandleW(nullptr));

/// command line arguments
inline const array<string<cat1>> args = [](int i) {
  auto a = ::CommandLineToArgvW(::GetCommandLineW(), &i);
  array<string<cat1>> r(i);
  for (nat j = 0; j < i; ++j) r[j] = cvt<cat1>(a[j]);
  ::LocalFree(a);
  return r;
}({});

/// main system logger
inline logger log{path(args[0]).replace_extension(".log")};

/// windows username
inline const string<cat1> username = [](unsigned long i) {
  ::GetUserNameW(nullptr, &i);
  auto s = string<cat2>(i, '\0');
  ::GetUserNameW(s.data(), &i);
  return cvt<cat1>(s);
}({});

} // namespace yw::main

export namespace yw {

/// window icon
class wicon {
  wicon(const wicon&) = delete;
  wicon& operator=(const wicon&) = delete;
protected:
  HICON hi{};
  wicon(HICON Icon) noexcept : hi(Icon) {}
public:
  /// predefined window icons
  class predefined;
  wicon(const predefined&) = delete;

  wicon() noexcept = default;
  wicon(wicon&& i) noexcept : hi(exchange(i.hi, nullptr)) {}
  explicit operator HICON() const noexcept { return hi; }
  explicit operator bool() const noexcept { return !hi; }

  ~wicon() noexcept {
    try {
      if (hi && !::DestroyIcon(hi)) main::log(logger::error, "failed to destroy icon");
    } catch (...) { main::log(logger::error, "failed to destroy icon"); }
  }

  wicon& operator=(wicon&& Icon) {
    if (hi && !::DestroyIcon(hi)) main::log(logger::error, "failed to destroy icon");
    return hi = exchange(Icon.hi, nullptr), *this;
  }

  /// constructs a window icon from a file
  [[nodiscard]] wicon(const path& File, const source& _ = {}) {
    auto bm = (HBITMAP)::LoadImageW(nullptr, File.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    if (!bm) {
      main::log(logger::error, "failed to load image for icon", _);
      return;
    }
    ICONINFO ii{.fIcon = true, .hbmMask = bm, .hbmColor = bm};
    hi = ::CreateIconIndirect(&ii);
    ::DeleteObject(bm);
    if (!hi) main::log(logger::error, "failed to create icon from file", _);
  }
};

static_assert(sizeof(wicon) == sizeof(HICON));

/// predefined window icons
class wicon::predefined : public wicon {
  template<nat I> predefined(constant<I>) noexcept //
    : wicon(::LoadIconW(0, reinterpret_cast<const cat2*>(I))) {}
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
};

inline const wicon::predefined wicon::predefined::app{constant<32512_n>{}};
inline const wicon::predefined wicon::predefined::error{constant<32513_n>{}};
inline const wicon::predefined wicon::predefined::question{constant<32514_n>{}};
inline const wicon::predefined wicon::predefined::warning{constant<32515_n>{}};
inline const wicon::predefined wicon::predefined::info{constant<32516_n>{}};
inline const wicon::predefined wicon::predefined::shield{constant<32518_n>{}};

/// window cursor
class wcursor {
  wcursor(const wcursor&) = delete;
  wcursor& operator=(const wcursor&) = delete;
protected:
  HCURSOR hc{};
  wcursor(HCURSOR Cursor) noexcept : hc(Cursor) {}
public:
  /// predefined window cursors
  class predefined;
  wcursor(const predefined&) = delete;

  wcursor() noexcept = default;
  wcursor(wcursor&& c) noexcept : hc(exchange(c.hc, nullptr)) {}
  explicit operator HCURSOR() const noexcept { return hc; }
  explicit operator bool() const noexcept { return !hc; }

  ~wcursor() noexcept {
    try {
      if (hc && !::DestroyCursor(hc)) main::log(logger::error, "failed to destroy cursor");
    } catch (...) { main::log(logger::error, "failed to destroy cursor"); }
  }

  wcursor& operator=(wcursor&& Cursor) {
    if (hc && !::DestroyCursor(hc)) main::log(logger::error, "failed to destroy cursor");
    return hc = exchange(Cursor.hc, nullptr), *this;
  }

  /// constructs a window cursor from a file
  [[nodiscard]] wcursor(const path& File, const source& _ = {}) {
    hc = ::LoadCursorFromFileW(File.c_str());
    if (!hc) main::log(logger::error, "failed to create cursor from file", _);
  }
};

static_assert(sizeof(wcursor) == sizeof(HCURSOR));

/// predefined window cursors
class wcursor::predefined : public wcursor {
  template<nat I> predefined(constant<I>) noexcept //
    : wcursor(::LoadCursorW(0, reinterpret_cast<const cat2*>(I))) {}
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
  static const wcursor::predefined sizenesw;
  static const wcursor::predefined sizewe;
  static const wcursor::predefined sizens;
  static const wcursor::predefined sizeall;
  static const wcursor::predefined no;
  static const wcursor::predefined hand;
  static const wcursor::predefined appstarting;
  static const wcursor::predefined help;
};

inline const wcursor::predefined wcursor::predefined::arrow{constant<32512_n>{}};
inline const wcursor::predefined wcursor::predefined::ibeam{constant<32513_n>{}};
inline const wcursor::predefined wcursor::predefined::wait{constant<32514_n>{}};
inline const wcursor::predefined wcursor::predefined::cross{constant<32515_n>{}};
inline const wcursor::predefined wcursor::predefined::uparrow{constant<32516_n>{}};
inline const wcursor::predefined wcursor::predefined::sizenesw{constant<32643_n>{}};
inline const wcursor::predefined wcursor::predefined::sizewe{constant<32644_n>{}};
inline const wcursor::predefined wcursor::predefined::sizens{constant<32645_n>{}};
inline const wcursor::predefined wcursor::predefined::sizeall{constant<32646_n>{}};
inline const wcursor::predefined wcursor::predefined::no{constant<32648_n>{}};
inline const wcursor::predefined wcursor::predefined::hand{constant<32649_n>{}};
inline const wcursor::predefined wcursor::predefined::appstarting{constant<32650_n>{}};
inline const wcursor::predefined wcursor::predefined::help{constant<32651_n>{}};

/// window messages
enum class wmessage {
  wm_null = WM_NULL,
  wm_create = WM_CREATE,
  wm_destroy = WM_DESTROY,
  wm_move = WM_MOVE,
  wm_size = WM_SIZE,
  wm_activate = WM_ACTIVATE,
  wm_setfocus = WM_SETFOCUS,
  wm_killfocus = WM_KILLFOCUS,
  wm_enable = WM_ENABLE,
  wm_setredraw = WM_SETREDRAW,
  wm_settext = WM_SETTEXT,
  wm_gettext = WM_GETTEXT,
  wm_gettextlength = WM_GETTEXTLENGTH,
  wm_paint = WM_PAINT,
  wm_close = WM_CLOSE,
  wm_queryendsession = WM_QUERYENDSESSION,
  wm_queryopen = WM_QUERYOPEN,
  wm_endsession = WM_ENDSESSION,
  wm_quit = WM_QUIT,
  wm_erasebkgnd = WM_ERASEBKGND,
  wm_syscolorchange = WM_SYSCOLORCHANGE,
  wm_showwindow = WM_SHOWWINDOW,
  wm_wininichange = WM_WININICHANGE,
  wm_settingchange = WM_SETTINGCHANGE,
  wm_devmodechange = WM_DEVMODECHANGE,
  wm_activateapp = WM_ACTIVATEAPP,
  wm_fontchange = WM_FONTCHANGE,
  wm_timechange = WM_TIMECHANGE,
  wm_cancelsmode = WM_CANCELMODE,
  wm_setcursor = WM_SETCURSOR,
  wm_mouseactivate = WM_MOUSEACTIVATE,
  wm_childactivate = WM_CHILDACTIVATE,
  wm_queuesync = WM_QUEUESYNC,
  wm_getminmaxinfo = WM_GETMINMAXINFO,
  wm_painticon = WM_PAINTICON,
  wm_iconerasebkgnd = WM_ICONERASEBKGND,
  wm_nextdlgctl = WM_NEXTDLGCTL,
  wm_spoolerstatus = WM_SPOOLERSTATUS,
  wm_drawitem = WM_DRAWITEM,
  wm_measureitem = WM_MEASUREITEM,
  wm_deleteitem = WM_DELETEITEM,
  wm_vkeytoitem = WM_VKEYTOITEM,
  wm_chartoitem = WM_CHARTOITEM,
  wm_setfont = WM_SETFONT,
  wm_getfont = WM_GETFONT,
  wm_sethotkey = WM_SETHOTKEY,
  wm_gethotkey = WM_GETHOTKEY,
  wm_querydragicon = WM_QUERYDRAGICON,
  wm_compareitem = WM_COMPAREITEM,
  wm_getobject = WM_GETOBJECT,
  wm_compacting = WM_COMPACTING,
  wm_commnotify = WM_COMMNOTIFY,
  wm_windowposchanging = WM_WINDOWPOSCHANGING,
  wm_windowposchanged = WM_WINDOWPOSCHANGED,
  wm_power = WM_POWER,
  wm_copydata = WM_COPYDATA,
  wm_canceljournal = WM_CANCELJOURNAL,
  wm_notify = WM_NOTIFY,
  wm_inputlangchangerequest = WM_INPUTLANGCHANGEREQUEST,
  wm_inputlangchange = WM_INPUTLANGCHANGE,
  wm_tcard = WM_TCARD,
  wm_help = WM_HELP,
  wm_userchanged = WM_USERCHANGED,
  wm_notifyformat = WM_NOTIFYFORMAT,
  wm_contextmenu = WM_CONTEXTMENU,
  wm_stylechanging = WM_STYLECHANGING,
  wm_stylechanged = WM_STYLECHANGED,
  wm_displaychange = WM_DISPLAYCHANGE,
  wm_geticon = WM_GETICON,
  wm_seticon = WM_SETICON,
  wm_nccreate = WM_NCCREATE,
  wm_ncdestroy = WM_NCDESTROY,
  wm_nccalcsize = WM_NCCALCSIZE,
  wm_nchittest = WM_NCHITTEST,
  wm_ncpaint = WM_NCPAINT,
  wm_ncactivate = WM_NCACTIVATE,
  wm_getdlgcode = WM_GETDLGCODE,
  wm_syncpaint = WM_SYNCPAINT,
  wm_ncmousemove = WM_NCMOUSEMOVE,
  wm_nclbuttondown = WM_NCLBUTTONDOWN,
  wm_nclbuttonup = WM_NCLBUTTONUP,
  wm_nclbuttondblclk = WM_NCLBUTTONDBLCLK,
  wm_ncrbuttondown = WM_NCRBUTTONDOWN,
  wm_ncrbuttonup = WM_NCRBUTTONUP,
  wm_ncrbuttondblclk = WM_NCRBUTTONDBLCLK,
  wm_ncmbuttondown = WM_NCMBUTTONDOWN,
  wm_ncmbuttonup = WM_NCMBUTTONUP,
  wm_ncmbuttondblclk = WM_NCMBUTTONDBLCLK,
  wm_ncxbuttondown = WM_NCXBUTTONDOWN,
  wm_ncxbuttonup = WM_NCXBUTTONUP,
  wm_ncxbuttondblclk = WM_NCXBUTTONDBLCLK,
  wm_input_device_change = WM_INPUT_DEVICE_CHANGE,
  wm_input = WM_INPUT,
  wm_keyfirst = WM_KEYFIRST,
  wm_keydown = WM_KEYDOWN,
  wm_keyup = WM_KEYUP,
  wm_char = WM_CHAR,
  wm_deadchar = WM_DEADCHAR,
  wm_syskeydown = WM_SYSKEYDOWN,
  wm_syskeyup = WM_SYSKEYUP,
  wm_syschar = WM_SYSCHAR,
  wm_sysdeadchar = WM_SYSDEADCHAR,
  wm_unichar = WM_UNICHAR,
  wm_keylast = WM_KEYLAST,
  wm_ime_startcomposition = WM_IME_STARTCOMPOSITION,
  wm_ime_endcomposition = WM_IME_ENDCOMPOSITION,
  wm_ime_composition = WM_IME_COMPOSITION,
  wm_ime_keylast = WM_IME_KEYLAST,
  wm_initdialog = WM_INITDIALOG,
  wm_command = WM_COMMAND,
  wm_syscommand = WM_SYSCOMMAND,
  wm_timer = WM_TIMER,
  wm_hscroll = WM_HSCROLL,
  wm_vscroll = WM_VSCROLL,
  wm_initmenu = WM_INITMENU,
  wm_initmenupopup = WM_INITMENUPOPUP,
  wm_gesture = WM_GESTURE,
  wm_gesturenotify = WM_GESTURENOTIFY,
  wm_menuselect = WM_MENUSELECT,
  wm_menuchar = WM_MENUCHAR,
  wm_enteridle = WM_ENTERIDLE,
  wm_menurbuttonup = WM_MENURBUTTONUP,
  wm_menudrag = WM_MENUDRAG,
  wm_menugetobject = WM_MENUGETOBJECT,
  wm_uninitmenupopup = WM_UNINITMENUPOPUP,
  wm_menucommand = WM_MENUCOMMAND,
  wm_changeuistate = WM_CHANGEUISTATE,
  wm_updateuistate = WM_UPDATEUISTATE,
  wm_queryuistate = WM_QUERYUISTATE,
  wm_ctlcolormsgbox = WM_CTLCOLORMSGBOX,
  wm_ctlcoloredit = WM_CTLCOLOREDIT,
  wm_ctlcolorlistbox = WM_CTLCOLORLISTBOX,
  wm_ctlcolorbtn = WM_CTLCOLORBTN,
  wm_ctlcolordlg = WM_CTLCOLORDLG,
  wm_ctlcolorscrollbar = WM_CTLCOLORSCROLLBAR,
  wm_ctlcolorstatic = WM_CTLCOLORSTATIC,
  mn_gethmenu = MN_GETHMENU,
  wm_mousefirst = WM_MOUSEFIRST,
  wm_mousemove = WM_MOUSEMOVE,
  wm_lbuttondown = WM_LBUTTONDOWN,
  wm_lbuttonup = WM_LBUTTONUP,
  wm_lbuttondblclk = WM_LBUTTONDBLCLK,
  wm_rbuttondown = WM_RBUTTONDOWN,
  wm_rbuttonup = WM_RBUTTONUP,
  wm_rbuttondblclk = WM_RBUTTONDBLCLK,
  wm_mbuttondown = WM_MBUTTONDOWN,
  wm_mbuttonup = WM_MBUTTONUP,
  wm_mbuttondblclk = WM_MBUTTONDBLCLK,
  wm_mousewheel = WM_MOUSEWHEEL,
  wm_xbuttondown = WM_XBUTTONDOWN,
  wm_xbuttonup = WM_XBUTTONUP,
  wm_xbuttondblclk = WM_XBUTTONDBLCLK,
  wm_mousehwheel = WM_MOUSEHWHEEL,
  wm_mouselast = WM_MOUSELAST,
  wm_parentnotify = WM_PARENTNOTIFY,
  wm_entermenuloop = WM_ENTERMENULOOP,
  wm_exitmenuloop = WM_EXITMENULOOP,
  wm_nextmenu = WM_NEXTMENU,
  wm_sizing = WM_SIZING,
  wm_capturechanged = WM_CAPTURECHANGED,
  wm_moving = WM_MOVING,
  wm_powerbroadcast = WM_POWERBROADCAST,
  wm_devicechange = WM_DEVICECHANGE,
  wm_mdicreate = WM_MDICREATE,
  wm_mdidestroy = WM_MDIDESTROY,
  wm_mdiactivate = WM_MDIACTIVATE,
  wm_mdirestore = WM_MDIRESTORE,
  wm_mdinext = WM_MDINEXT,
  wm_mdimaximize = WM_MDIMAXIMIZE,
  wm_mditile = WM_MDITILE,
  wm_mdicascade = WM_MDICASCADE,
  wm_mdiiconarrange = WM_MDIICONARRANGE,
  wm_mdigetactive = WM_MDIGETACTIVE,
  wm_mdisetmenu = WM_MDISETMENU,
  wm_entersizemove = WM_ENTERSIZEMOVE,
  wm_exitsizemove = WM_EXITSIZEMOVE,
  wm_dropfiles = WM_DROPFILES,
  wm_mdirefreshmenu = WM_MDIREFRESHMENU,
  wm_pointerdevicechange = WM_POINTERDEVICECHANGE,
  wm_pointerdeviceinrange = WM_POINTERDEVICEINRANGE,
  wm_pointerdeviceoutofrange = WM_POINTERDEVICEOUTOFRANGE,
  wm_touch = WM_TOUCH,
  wm_ncpointerupdate = WM_NCPOINTERUPDATE,
  wm_ncpointerdown = WM_NCPOINTERDOWN,
  wm_ncpointerup = WM_NCPOINTERUP,
  wm_pointerupdate = WM_POINTERUPDATE,
  wm_pointerdown = WM_POINTERDOWN,
  wm_pointerup = WM_POINTERUP,
  wm_pointerenter = WM_POINTERENTER,
  wm_pointerleave = WM_POINTERLEAVE,
  wm_pointeractivate = WM_POINTERACTIVATE,
  wm_pointercapturechanged = WM_POINTERCAPTURECHANGED,
  wm_touchhittesting = WM_TOUCHHITTESTING,
  wm_pointerwheel = WM_POINTERWHEEL,
  wm_pointerhwheel = WM_POINTERHWHEEL,
  dm_pointerhittest = DM_POINTERHITTEST,
  wm_pointerroutedto = WM_POINTERROUTEDTO,
  wm_pointerroutedaway = WM_POINTERROUTEDAWAY,
  wm_pointerroutedreleased = WM_POINTERROUTEDRELEASED,
  wm_ime_setcontext = WM_IME_SETCONTEXT,
  wm_ime_notify = WM_IME_NOTIFY,
  wm_ime_control = WM_IME_CONTROL,
  wm_ime_compositionfull = WM_IME_COMPOSITIONFULL,
  wm_ime_select = WM_IME_SELECT,
  wm_ime_char = WM_IME_CHAR,
  wm_ime_request = WM_IME_REQUEST,
  wm_ime_keydown = WM_IME_KEYDOWN,
  wm_ime_keyup = WM_IME_KEYUP,
  wm_mousehover = WM_MOUSEHOVER,
  wm_mouseleave = WM_MOUSELEAVE,
  wm_ncmousehover = WM_NCMOUSEHOVER,
  wm_ncmouseleave = WM_NCMOUSELEAVE,
  wm_wtssession_change = WM_WTSSESSION_CHANGE,
  wm_tablet_first = WM_TABLET_FIRST,
  wm_tablet_last = WM_TABLET_LAST,
  wm_dpichanged = WM_DPICHANGED,
  wm_dpichanged_beforeparent = WM_DPICHANGED_BEFOREPARENT,
  wm_dpichanged_afterparent = WM_DPICHANGED_AFTERPARENT,
  wm_getdpiscaledsize = WM_GETDPISCALEDSIZE,
  wm_cut = WM_CUT,
  wm_copy = WM_COPY,
  wm_paste = WM_PASTE,
  wm_clear = WM_CLEAR,
  wm_undo = WM_UNDO,
  wm_renderformat = WM_RENDERFORMAT,
  wm_renderallformats = WM_RENDERALLFORMATS,
  wm_destroyclipboard = WM_DESTROYCLIPBOARD,
  wm_drawclipboard = WM_DRAWCLIPBOARD,
  wm_paintclipboard = WM_PAINTCLIPBOARD,
  wm_vscrollclipboard = WM_VSCROLLCLIPBOARD,
  wm_sizeclipboard = WM_SIZECLIPBOARD,
  wm_askcbformatname = WM_ASKCBFORMATNAME,
  wm_changecbchain = WM_CHANGECBCHAIN,
  wm_hscrollclipboard = WM_HSCROLLCLIPBOARD,
  wm_querynewpalette = WM_QUERYNEWPALETTE,
  wm_paletteischanging = WM_PALETTEISCHANGING,
  wm_palettechanged = WM_PALETTECHANGED,
  wm_hotkey = WM_HOTKEY,
  wm_print = WM_PRINT,
  wm_printclient = WM_PRINTCLIENT,
  wm_appcommand = WM_APPCOMMAND,
  wm_themechanged = WM_THEMECHANGED,
  wm_clipboardupdate = WM_CLIPBOARDUPDATE,
  wm_dwmcompositionchanged = WM_DWMCOMPOSITIONCHANGED,
  wm_dwmncrenderingchanged = WM_DWMNCRENDERINGCHANGED,
  wm_dwmcolorizationcolorchanged = WM_DWMCOLORIZATIONCOLORCHANGED,
  wm_dwmwindowmaximizedchange = WM_DWMWINDOWMAXIMIZEDCHANGE,
  wm_dwmsendiconicthumbnail = WM_DWMSENDICONICTHUMBNAIL,
  wm_dwmsendiconiclivepreviewbitmap = WM_DWMSENDICONICLIVEPREVIEWBITMAP,
  wm_gettitlebarinfoex = WM_GETTITLEBARINFOEX,
  wm_handheldfirst = WM_HANDHELDFIRST,
  wm_handheldlast = WM_HANDHELDLAST,
  wm_affirst = WM_AFXFIRST,
  wm_aflast = WM_AFXLAST,
  wm_penwinfirst = WM_PENWINFIRST,
  wm_penwinlast = WM_PENWINLAST,
  wm_app = WM_APP,
  wm_user = WM_USER,
};
using enum wmessage;

/// class to manage a window
class window;

/// window procedure
template<invocable_r<bool, window&, wmessage, nat8, int8> Fn> //
class wprocedure : public Fn {
  inline static Fn fn{};
  static int8 CALLBACK wndproc(HWND hw, nat4 msg, nat8 wp, int8 lp) {
    list<HWND, vector2<int>, MSG> w{hw, {}, {hw, wmessage(msg), wp, lp}};
    if (fn(reinterpret_cast<window&>(w), wmessage(msg), wp, lp)) return 0;
    else return ::DefWindowProcW(hw, msg, wp, lp);
  }
public:
  explicit operator WNDPROC() const noexcept { return wndproc; }
};

/// window class
class wclass {
  wclass(const wclass&) = delete;
  wclass& operator=(const wclass&) = delete;
protected:
  string_view<cat1> _name{};
  HBRUSH _hbrush{};
  wclass(format_string<cat1> Name) noexcept : _name(Name.get()) {}
  HBRUSH create_solid_brush(const color& c) noexcept {
    auto a = intrin::mm_cvtps_epi32(xvmul(c, xv_constant<255>));
    a = intrin::mm_shuffle_epi8(a, xv_constant<0xc0080400>);
    //   static const auto f255 = _mm_set1_ps(255.0f);
    //   static const auto shuffle = _mm_set1_epi32(0xc0080400);
    //   auto a = _mm_loadu_ps(RGBA);
    //   auto b = _mm_cvtps_epi32(_mm_mul_ps(a, f255));
    //   b = _mm_shuffle_epi8(b, shuffle);
    //   return ::CreateSolidBrush(unsigned(_mm_cvtsi128_si32(b)));
  }
public:
  /// predefined window classe for controls
  class control;
  wclass(const control&) = delete;

  /// window class styles
  enum class style : nat4 {
    vertical_redraw = CS_VREDRAW,
    horizontal_redraw = CS_HREDRAW,
    double_clicks = CS_DBLCLKS,
    own_dc = CS_OWNDC,
    class_dc = CS_CLASSDC,
    parent_dc = CS_PARENTDC,
    no_close = CS_NOCLOSE,
    save_bits = CS_SAVEBITS,
    byte_align_client = CS_BYTEALIGNCLIENT,
    byte_align_window = CS_BYTEALIGNWINDOW,
    global_class = CS_GLOBALCLASS,
    drop_shadow = CS_DROPSHADOW,
  };
  using enum style;
  friend constexpr style operator|(style a, style b) noexcept { return style(nat4(a) | nat4(b)); }
  friend constexpr style operator&(style a, style b) noexcept { return style(nat4(a) & nat4(b)); }
  friend constexpr style operator^(style a, style b) noexcept { return style(nat4(a) ^ nat4(b)); }
  friend constexpr style operator~(style a) noexcept { return style(~nat4(a)); }

  wclass() noexcept = default;
  wclass(wclass&&) noexcept : _name(exchange(_name, {})), _hbrush(exchange(_hbrush, nullptr)) {}
  explicit operator const cat1*() const noexcept { return _name.data(); }
  explicit operator bool() const noexcept { return !_name.empty(); }

  ~wclass() noexcept {
    try {
      if (_hbrush && !::DeleteObject(_hbrush)) main::log(logger::error, "failed to delete brush");
      if (auto name = cvt<cat2>(_name); !name.empty())
        if (!::UnregisterClassW(name.data(), (HINSTANCE)main::hinstance)) //
          main::log(logger::error, "failed to unregister window class");
    } catch (...) { main::log(logger::error, "failed to unregister window class"); }
  }

  wclass& operator=(wclass&& C) {
    if (_hbrush && !::DeleteObject(_hbrush)) main::log(logger::error, "failed to delete brush");
    if (auto name = cvt<cat2>(_name); !name.empty())
      if (!::UnregisterClassW(name.data(), (HINSTANCE)main::hinstance)) //
        main::log(logger::error, "failed to unregister window class");
  }

  /// creates and registers window class
  template<typename Fn> [[nodiscard]] wclass(                                        //
    const format_string<cat1> Name, const wprocedure<Fn>& Proc, wclass::style Style, //
    const color& Color = color::white, const wicon& Icon = wicon::predefined::app,   //
    const wcursor& Cursor = wcursor::predefined::arrow, const source& _ = {})
    : _name(Name.get()), _hbrush(::CreateSolidBrush(Color.r)) {
    try {
      if (auto name = cvt<cat2>(_name); !name.empty()) {
        if (!::RegisterClassW(&(WNDCLASSW){.style = nat4(Style), .lpfnWndProc = (WNDPROC)Proc, .hInstance = (HINSTANCE)main::hinstance, .hIcon = (HICON)Icon, .hCursor = (HCURSOR)Cursor, .hbrBackground = _hbrush, .lpszClassName = name.data()})) throw std::runtime_error("failed to register window class");
      }
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to creates and registers window class", _);
      if (_hbrush && !::DeleteObject(exchange(_hbrush, nullptr))) //
        main::log(logger::error, "failed to delete brush", _);
      _name = {};
    }
  }
};

} // namespace yw
