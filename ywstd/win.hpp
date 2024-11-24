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

export namespace win { // types
using hbitmap = ::HBITMAP;
using hbrush = ::HBRUSH;
using hcursor = ::HCURSOR;
using hgdiobj = ::HGDIOBJ;
using hicon = ::HICON;
using hinstance = ::HINSTANCE;
using hmenu = ::HMENU;
using hpen = ::HPEN;
using hwnd = ::HWND;

struct point {
  int x, y;
};
static_assert(sizeof(point) == sizeof(POINT));

struct rect {
  int left, top, right, bottom;
};
static_assert(sizeof(rect) == sizeof(RECT));
}

export namespace win { // D

/// \brief dialog box commands
enum class dialog_box_command : int {
  id_ok = 1,
  id_cancel = 2,
  id_abort = 3,
  id_retry = 4,
  id_ignore = 5,
  id_yes = 6,
  id_no = 7,
  id_close = 8,
  id_help = 9,
  id_try_again = 10,
  id_continue = 11,
  id_timeout = 32000,
};
using enum dialog_box_command;
constexpr bool operator==(dialog_box_command a, int b) noexcept { return int(a) == b; }
constexpr bool operator==(int a, dialog_box_command b) noexcept { return a == int(b); }
constexpr auto operator<=>(dialog_box_command a, int b) noexcept { return int(a) <=> b; }
constexpr auto operator<=>(int a, dialog_box_command b) noexcept { return a <=> int(b); }
}

export namespace win { // G

/// \brief gets the command line for the current process
inline wchar_t* get_command_line() noexcept { return ::GetCommandLineW(); }

/// \brief gets a module handle
inline hinstance get_module_handle(const wchar_t* ModuleName) noexcept { return ::GetModuleHandleW(ModuleName); }
}

export namespace win { // L

/// \brief standard cursor ids
enum class cursor_id : unsigned long long {
  idc_arrow = 32512,
  idc_ibeam = 32513,
  idc_wait = 32514,
  idc_cross = 32515,
  idc_uparrow = 32516,
  idc_size = 32640, // obsolete
  idc_icon = 32641, // obsolete
  idc_sizenwse = 32642,
  idc_sizenesw = 32643,
  idc_sizewe = 32644,
  idc_sizens = 32645,
  idc_sizeall = 32646,
  idc_no = 32648,
  idc_hand = 32649,
  idc_appstarting = 32650,
  idc_help = 32651,
  idc_pin = 32671,
  idc_person = 32672,
};
using enum cursor_id;

/// \brief loads a cursor
inline hcursor load_cursor(hinstance Instance, const wchar_t* CursorName) noexcept { return ::LoadCursorW(Instance, CursorName); }
/// \brief loads a standard cursor
inline hcursor load_cursor(cursor_id Id) noexcept { return ::LoadCursorW(nullptr, (const wchar_t*)unsigned long long(Id)); }

/// \brief standard icon ids
enum class icon_id : unsigned long long {
  idi_application = 32512,
  idi_hand = 32513,
  idi_question = 32514,
  idi_exclamation = 32515,
  idi_asterisk = 32516,
  idi_winlogo = 32517,
  idi_shield = 32518,
  idi_warning = idi_exclamation,
  idi_error = idi_hand,
  idi_information = idi_asterisk,
};
using enum icon_id;

/// \brief loads an icon
inline hicon load_icon(hinstance Instance, const wchar_t* IconName) noexcept { return ::LoadIconW(Instance, IconName); }
/// \brief loads a standard icon
inline hicon load_icon(icon_id Id) noexcept { return ::LoadIconW(nullptr, (const wchar_t*)unsigned long long(Id)); }
}

export namespace win { // M
/// \brief parameters for `message_box`
enum class message_box_type : unsigned {
  mb_ok = 0x00000000,
  mb_ok_cancel = 0x00000001,
  mb_abort_retry_ignore = 0x00000002,
  mb_yes_no_cancel = 0x00000003,
  mb_yes_no = 0x00000004,
  mb_retry_cancel = 0x00000005,
  mb_cancel_try_continue = 0x00000006,
  mb_icon_hand = 0x00000010,
  mb_icon_question = 0x00000020,
  mb_icon_exclamation = 0x00000030,
  mb_icon_asterisk = 0x00000040,
  mb_user_icon = 0x00000080,
  mb_icon_warning = mb_icon_exclamation,
  mb_icon_error = mb_icon_hand,
  mb_icon_information = mb_icon_asterisk,
  mb_icon_stop = mb_icon_hand,
  mb_def_button1 = 0x00000000,
  mb_def_button2 = 0x00000100,
  mb_def_button3 = 0x00000200,
  mb_def_button4 = 0x00000300,
  mb_appl_modal = 0x00000000,
  mb_system_modal = 0x00001000,
  mb_task_modal = 0x00002000,
  mb_help = 0x00004000,
  mb_no_focus = 0x00008000,
  mb_set_foreground = 0x00010000,
  mb_default_desktop_only = 0x00020000,
  mb_topmost = 0x00040000,
  mb_right = 0x00080000,
  mb_rtl_reading = 0x00100000,
  mb_service_notification = 0x00200000,
  mb_service_notification_nt3x = 0x00040000,
  mb_type_mask = 0x0000000F,
  mb_icon_mask = 0x000000F0,
  mb_def_mask = 0x00000F00,
  mb_mode_mask = 0x00003000,
  mb_misc_mask = 0x0000C000,
};
using enum message_box_type;
constexpr auto operator|(message_box_type a, message_box_type b) noexcept { return message_box_type((unsigned)a | (unsigned)b); }
constexpr auto operator&(message_box_type a, message_box_type b) noexcept { return message_box_type((unsigned)a & (unsigned)b); }
constexpr auto operator^(message_box_type a, message_box_type b) noexcept { return message_box_type((unsigned)a ^ (unsigned)b); }
constexpr auto operator~(message_box_type a) noexcept { return message_box_type(~(unsigned)a); }
/// \brief result of `message_box`
using message_box_result = dialog_box_command;
/// \brief displays a message box
inline message_box_result message_box(hwnd hw, const wchar_t* Text, const wchar_t* Caption, message_box_type Type) noexcept { //
  return message_box_result(::MessageBoxW(hw, Text, Caption, unsigned(Type)));
}
}

export namespace win { // related to message

/// \brief window messages
enum class window_message : unsigned {
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
using enum window_message;
constexpr auto operator|(window_message a, window_message b) noexcept { return window_message((unsigned)a | (unsigned)b); }
constexpr auto operator&(window_message a, window_message b) noexcept { return window_message((unsigned)a & (unsigned)b); }
constexpr auto operator^(window_message a, window_message b) noexcept { return window_message((unsigned)a ^ (unsigned)b); }
constexpr auto operator~(window_message a) noexcept { return window_message(~(unsigned)a); }

/// \brief default window procedure
inline long long def_window_proc(hwnd hw, window_message msg, unsigned long long wp, long long lp) noexcept { return ::DefWindowProcW(hw, unsigned(msg), wp, lp); }

/// \brief posts quit message
inline void post_quit_message(int ExitCode) noexcept { ::PostQuitMessage(ExitCode); }
}

export namespace win { // related to window class

/// \brief window class styles
enum class class_style : unsigned {
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
using enum class_style;
constexpr auto operator|(class_style a, class_style b) noexcept { return class_style((unsigned)a | (unsigned)b); }
constexpr auto operator&(class_style a, class_style b) noexcept { return class_style((unsigned)a & (unsigned)b); }
constexpr auto operator^(class_style a, class_style b) noexcept { return class_style((unsigned)a ^ (unsigned)b); }
constexpr auto operator~(class_style a) noexcept { return class_style(~(unsigned)a); }

/// \brief window procedure type
using wndproc = long long(CALLBACK*)(hwnd, window_message, unsigned long long, long long);

/// \brief window class
struct wndclass {
  unsigned size{};
  class_style style{};
  wndproc proc{};
  int cls_extra{};
  int wnd_extra{};
  hinstance instance{};
  hicon icon{};
  hcursor cursor{};
  hbrush background{};
  const wchar_t* menu{};
  const wchar_t* name{};
  hicon small_icon{};
  wndclass() noexcept = default;
  wndclass(const wchar_t* ClassName, wndproc WndProc, class_style Style) noexcept //
    : size(sizeof(wndclass)), style(Style), proc(WndProc), instance(get_module_handle(nullptr)), cursor((HCURSOR)load_cursor(idc_arrow)), name(ClassName) {}
};
static_assert(sizeof(wndclass) == sizeof(WNDCLASSEXW));

/// \brief registers a window class
inline unsigned short register_class(const wndclass& WndClass) noexcept { return ::RegisterClassExW((WNDCLASSEXW*)&WndClass); }

/// \brief unregisters a window class
inline bool unregister_class(const wchar_t* ClassName, hinstance Instance) noexcept { return ::UnregisterClassW(ClassName, Instance); }
}
/*
export namespace win { // related to drawing

////////////////////////////////////////////////////////////////////////////////
// DEVICE CONTEXT

using hdc = ::HDC;

/// \brief gets the device context for a window
inline hdc get_dc(hwnd hw) noexcept { return ::GetDC(hw); }

/// \brief releases a device context
inline bool release_dc(hwnd hw, hdc dc) noexcept { return ::ReleaseDC(hw, dc); }

/// \brief saves the device context
inline int save_dc(hdc dc) noexcept { return ::SaveDC(dc); }

/// \brief restores the device context
inline bool restore_dc(hdc dc, int State) noexcept { return ::RestoreDC(dc, State); }

////////////////////////////////////////////////////////////////////////////////
// PAINTING

struct paintstruct {
  win::hdc hdc{};
  bool erase{};
  win::rect paint{};
private:
  int restore{};
  int inc_update{};
  unsigned char rgb_reserved[32]{};
};
static_assert(sizeof(paintstruct) == sizeof(PAINTSTRUCT));

/// \brief begins painting in a window
inline hdc begin_paint(hwnd hw, paintstruct& ps) noexcept { return ::BeginPaint(hw, (PAINTSTRUCT*)&ps); }

/// \brief ends painting in a window
inline bool end_paint(hwnd hw, const paintstruct& ps) noexcept { return ::EndPaint(hw, (const PAINTSTRUCT*)&ps); }

////////////////////////////////////////////////////////////////////////////////
// TEXT

/// \brief outputs a text string to a device context
inline bool text_out(hdc dc, int X, int Y, const wchar_t* Textl, int Count) noexcept { return ::TextOutW(dc, X, Y, Textl, Count); }

/// \brief flags for `draw_text`
enum class draw_text_flag : unsigned {
  dt_top = 0x00000000,
  dt_left = 0x00000000,
  dt_center = 0x00000001,
  dt_right = 0x00000002,
  dt_vcenter = 0x00000004,
  dt_bottom = 0x00000008,
  dt_wordbreak = 0x00000010,
  dt_singleline = 0x00000020,
  dt_expandtabs = 0x00000040,
  dt_tabstop = 0x00000080,
  dt_nocalcwidth = 0x00000100,
  dt_externalleading = 0x00000200,
  dt_calcrect = 0x00000400,
  dt_noprefix = 0x00000800,
  dt_internal = 0x00001000,
  dt_editcontrol = 0x00002000,
  dt_path_ellipsis = 0x00004000,
  dt_end_ellipsis = 0x00008000,
  dt_modifystring = 0x00010000,
  dt_rtlreading = 0x00020000,
  dt_word_ellipsis = 0x00040000,
  dt_no_full_width_char_break = 0x00080000,
  dt_hide_prefix = 0x00100000,
  dt_prefix_only = 0x00200000,
};
using enum draw_text_flag;
constexpr auto operator|(draw_text_flag a, draw_text_flag b) noexcept { return draw_text_flag((unsigned)a | (unsigned)b); }
constexpr auto operator&(draw_text_flag a, draw_text_flag b) noexcept { return draw_text_flag((unsigned)a & (unsigned)b); }
constexpr auto operator^(draw_text_flag a, draw_text_flag b) noexcept { return draw_text_flag((unsigned)a ^ (unsigned)b); }
constexpr auto operator~(draw_text_flag a) noexcept { return draw_text_flag(~(unsigned)a); }

/// \brief draws text in the specified rectangle
inline int draw_text(hdc dc, const wchar_t* Text, int Count, rect& Rect, draw_text_flag Format) noexcept { return ::DrawTextW(dc, Text, Count, (RECT*)&Rect, unsigned(Format)); }

struct textmetric {
  int height{};
  int ascent{};
  int descent{};
  int internal_leading{};
  int external_leading{};
  int ave_char_width{};
  int max_char_width{};
  int weight{};
  int overhang{};
  int digitized_aspect_x{};
  int digitized_aspect_y{};
  wchar_t first_char{};
  wchar_t last_char{};
  wchar_t default_char{};
  wchar_t break_char{};
  unsigned char italic{};
  unsigned char underlined{};
  unsigned char struck_out{};
  unsigned char pitch_and_family{};
  unsigned char char_set{};
};
static_assert(sizeof(textmetric) == sizeof(TEXTMETRICW));

/// \brief gets text metrics
inline bool get_text_metrics(hdc dc, textmetric& tm) noexcept { return ::GetTextMetricsW(dc, (TEXTMETRICW*)&tm); }

/// \brief sets the text color; returns the previous color
inline unsigned set_text_color(hdc dc, unsigned Color) noexcept { return ::SetTextColor(dc, Color); }

/// \brief gets the text color
inline unsigned get_text_color(hdc dc) noexcept { return ::GetTextColor(dc); }

/// \brief sets the text background color; returns the previous color
inline unsigned set_text_background_color(hdc dc, unsigned Color) noexcept { return ::SetBkColor(dc, Color); }

/// \brief gets the text background color
inline unsigned get_text_background_color(hdc dc) noexcept { return ::GetBkColor(dc); }

/// \brief sets the text background mode; returns the previous mode
inline bool set_text_background_mode(hdc dc, bool Opaque) noexcept { return ::SetBkMode(dc, 1 + Opaque) == 2; }

/// \brief gets the text background mode; returns true if opaque
inline bool get_text_background_mode(hdc dc) noexcept { return ::GetBkMode(dc) == 2; }

////////////////////////////////////////////////////////////////////////////////
// DRAWING

/// \brief sets the color of a pixel; returns the previous color
inline unsigned set_pixel(hdc dc, int X, int Y, unsigned Color) noexcept { return ::SetPixel(dc, X, Y, Color); }

/// \brief gets the color of a pixel
inline unsigned get_pixel(hdc dc, int X, int Y) noexcept { return ::GetPixel(dc, X, Y); }

/// \brief moves the current position to draw line
inline bool move_to(hdc dc, int X, int Y) noexcept { return ::MoveToEx(dc, X, Y, nullptr); }

/// \brief moves the current position to draw line;
inline bool move_to(hdc dc, int X, int Y, point& Prev) noexcept { return ::MoveToEx(dc, X, Y, (POINT*)&Prev); }

/// \brief draws a line
inline bool line_to(hdc dc, int X, int Y) noexcept { return ::LineTo(dc, X, Y); }

/// \brief draws a polyline
inline bool polyline(hdc dc, const point* Points, int Count) noexcept { return ::Polyline(dc, (const POINT*)Points, Count); }

/// \brief fills a polygon
inline bool polygon(hdc dc, const point* Points, int Count) noexcept { return ::Polygon(dc, (const POINT*)Points, Count); }

/// \brief fills polygons
inline bool poly_polygon(hdc dc, const point* Points, const int* Counts, int Count) noexcept { return ::PolyPolygon(dc, (const POINT*)Points, Counts, Count); }

/// \brief fills a rectangle
inline bool rectangle(hdc dc, int Left, int Top, int Right, int Bottom) noexcept { return ::Rectangle(dc, Left, Top, Right, Bottom); }

/// \brief fills a ellipse
inline bool ellipse(hdc dc, int Left, int Top, int Right, int Bottom) noexcept { return ::Ellipse(dc, Left, Top, Right, Bottom); }

/// \brief fills a round rectangle
inline bool round_rect(hdc dc, int Left, int Top, int Right, int Bottom, int Width, int Height) noexcept { return ::RoundRect(dc, Left, Top, Right, Bottom, Width, Height); }

/// \brief draws an arc
inline bool arc(hdc dc, int Left, int Top, int Right, int Bottom, int StartX, int StartY, int EndX, int EndY) noexcept { return ::Arc(dc, Left, Top, Right, Bottom, StartX, StartY, EndX, EndY); }

/// \brief fills a pie
inline bool pie(hdc dc, int Left, int Top, int Right, int Bottom, int StartX, int StartY, int EndX, int EndY) noexcept { return ::Pie(dc, Left, Top, Right, Bottom, StartX, StartY, EndX, EndY); }

/// \brief fills a chord
inline bool chord(hdc dc, int Left, int Top, int Right, int Bottom, int StartX, int StartY, int EndX, int EndY) noexcept { return ::Chord(dc, Left, Top, Right, Bottom, StartX, StartY, EndX, EndY); }

/// \brief draws bezier curves
inline bool poly_bezier(hdc dc, const point* Points, int Count) noexcept { return ::PolyBezier(dc, (const POINT*)Points, Count); }

/// \brief draws bezier curves starting from the current position
inline bool poly_bezier_to(hdc dc, const point* Points, int Count) noexcept { return ::PolyBezierTo(dc, (const POINT*)Points, Count); }

/// \brief returns the color code for WIN32
inline unsigned rgb(unsigned char R, unsigned char G, unsigned char B) noexcept { return unsigned(R) | unsigned(G) << 8 | unsigned(B) << 16; }

////////////////////////////////////////////////////////////////////////////////
// STOCK OBJECT

/// \brief stock objects
enum class stock_object : int {
  white_brush = 0,
  ltgray_brush = 1,
  gray_brush = 2,
  dkgray_brush = 3,
  black_brush = 4,
  null_brush = 5,
  hollow_brush = null_brush,
  white_pen = 6,
  black_pen = 7,
  null_pen = 8,
  oem_fixed_font = 10,
  ansi_fixed_font = 11,
  ansi_var_font = 12,
  system_font = 13,
  device_default_font = 14,
  default_palette = 15,
  system_fixed_font = 16,
  default_gui_font = 17,
  dc_brush = 18,
  dc_pen = 19,
};
using enum stock_object;

/// \brief gets a stock object
inline hgdiobj get_stock_object(stock_object Object) noexcept { return ::GetStockObject(int(Object)); }

/// \brief selects an object into a device context
inline hgdiobj select_object(hdc dc, hgdiobj Object) noexcept { return ::SelectObject(dc, Object); }

/// \brief deletes a GDI object created by `create_*`
inline bool delete_object(hgdiobj Object) noexcept { return ::DeleteObject(Object); }

////////////////////////////////////////////////////////////////////////////////
// PEN

/// \brief pen styles
enum class pen_style : int {
  ps_solid = 0,
  ps_dash = 1,
  ps_dot = 2,
  ps_dashdot = 3,
  ps_dashdotdot = 4,
  ps_null = 5,
  ps_insideframe = 6,
  ps_userstyle = 7,
  ps_alternate = 8,
  ps_endcap_round = 0x00000000,
  ps_endcap_square = 0x00000100,
  ps_endcap_flat = 0x00000200,
  ps_join_round = 0x00000000,
  ps_join_bevel = 0x00001000,
  ps_join_miter = 0x00002000,
  ps_cosmetic = 0x00000000,
  ps_geometric = 0x00010000,
};
using enum pen_style;
constexpr auto operator|(pen_style a, pen_style b) noexcept { return pen_style((int)a | (int)b); }
constexpr auto operator&(pen_style a, pen_style b) noexcept { return pen_style((int)a & (int)b); }
constexpr auto operator^(pen_style a, pen_style b) noexcept { return pen_style((int)a ^ (int)b); }
constexpr auto operator~(pen_style a) noexcept { return pen_style(~(int)a); }

/// \brief creates a pen
inline hpen create_pen(pen_style Style, int Width, unsigned Color) noexcept { return (hpen)::CreatePen(int(Style), Width, Color); }

////////////////////////////////////////////////////////////////////////////////
// BRUSH

/// \brief creates a solid brush
inline hbrush create_solid_brush(unsigned Color) noexcept { return (hbrush)::CreateSolidBrush(Color); }

/// \brief hatch styles
enum class hatch_style : int {
  hs_horizontal = 0,
  hs_vertical = 1,
  hs_fdiagonal = 2,
  hs_bdiagonal = 3,
  hs_cross = 4,
  hs_diagcross = 5,
};
using enum hatch_style;

/// \brief creates a hatch brush
inline hbrush create_hatch_brush(hatch_style Style, unsigned Color) noexcept { return (hbrush)::CreateHatchBrush(int(Style), Color); }

/// \brief creates a pattern brush
inline hgdiobj create_pattern_brush(hbitmap Bitmap) noexcept { return ::CreatePatternBrush(Bitmap); }

////////////////////////////////////////////////////////////////////////////////
// FONT

/// \brief creates a font
inline hgdiobj create_font(int Height, int Width, int Escapement, int Orientation, int Weight, unsigned char Italic, unsigned char Underline, unsigned char StrikeOut, unsigned char CharSet, unsigned char OutPrecision, unsigned char ClipPrecision, unsigned char Quality, unsigned char PitchAndFamily, const wchar_t* FaceName) noexcept { //
  return ::CreateFontW(Height, Width, Escapement, Orientation, Weight, Italic, Underline, StrikeOut, CharSet, OutPrecision, ClipPrecision, Quality, PitchAndFamily, FaceName);
}

/// \brief creates a font
inline hgdiobj create_font(const wchar_t* FaceName, int Height, int Weight, bool Italic, bool Underline, bool StrikeOut) noexcept { //
  return create_font(Height, 0, 0, 0, Weight, Italic, Underline, StrikeOut, 0, 0, 0, 0, 0, FaceName);
}

////////////////////////////////////////////////////////////////////////////////
// RASTER OPERATION

/// \brief raster operation modes
enum class raster_operation_mode : int {
  ro_black = 1,
  ro_notmergepen = 2,
  ro_masknotpen = 3,
  ro_notcopypen = 4,
  ro_maskpennot = 5,
  ro_not = 6,
  ro_xorpen = 7,
  ro_notmaskpen = 8,
  ro_maskpen = 9,
  ro_notxorpen = 10,
  ro_nop = 11,
  ro_mergenotpen = 12,
  ro_copypen = 13,
  ro_mergepennot = 14,
  ro_mergepen = 15,
  ro_white = 16,
};
using enum raster_operation_mode;

/// \brief sets the raster operation mode
inline raster_operation_mode set_raster_operation(hdc dc, raster_operation_mode Mode) noexcept { return ::SetROP2(dc, int(Mode)); }

}
*/
export namespace win { // related to window

/// \brief window styles; upper 32 bits are extended styles
enum class window_style : unsigned long long {
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
using enum window_style;
constexpr auto operator|(window_style a, window_style b) noexcept { return window_style((unsigned long long)a | (unsigned long long)b); }
constexpr auto operator&(window_style a, window_style b) noexcept { return window_style((unsigned long long)a & (unsigned long long)b); }
constexpr auto operator^(window_style a, window_style b) noexcept { return window_style((unsigned long long)a ^ (unsigned long long)b); }
constexpr auto operator~(window_style a) noexcept { return window_style(~(unsigned long long)a); }

/// \brief creates a window
inline hwnd create_window(const wchar_t* ClassName, const wchar_t* WindowName, window_style Style, int X, int Y, //
                          int Width, int Height, hwnd Parent, hmenu Menu, hinstance Instance, void* Param) noexcept {
  auto style = unsigned((unsigned long long)Style & 0xffffffff);
  auto exstyle = unsigned((unsigned long long)Style >> 32);
  return ::CreateWindowExW(exstyle, ClassName, WindowName, style, X, Y, Width, Height, Parent, Menu, Instance, Param);
}

/// \brief destroys a window
inline bool destroy_window(hwnd hw) noexcept { return ::DestroyWindow(hw); }

////////////////////////////////////////////////////////////////////////////////
// RECT

/// \brief gets the client area of a window
inline bool get_client_rect(hwnd hw, rect& rc) noexcept { return ::GetClientRect(hw, (RECT*)&rc); }

/// \brief gets the window rectangle
inline bool get_window_rect(hwnd hw, rect& rc) noexcept { return ::GetWindowRect(hw, (RECT*)&rc); }

/// \brief adjusts the window rectangle
inline bool adjust_window_rect(hwnd hw, rect& rc, window_style Style, bool Menu) noexcept {
  auto style = unsigned(unsigned long long(Style) & 0xffffffff);
  auto exstyle = unsigned(unsigned long long(Style) >> 32);
  return ::AdjustWindowRectEx((RECT*)&rc, style, Menu, exstyle);
}

/// \brief parameters for `show_window`
enum class show_window_command : int {
  sw_hide = 0,
  sw_show_normal = 1,
  sw_show_minimized = 2,
  sw_show_maximized = 3,
  sw_show_no_activate = 4,
  sw_show = 5,
  sw_minimize = 6,
  sw_show_min_no_activate = 7,
  sw_show_na = 8,
  sw_restore = 9,
  sw_show_default = 10,
  sw_force_minimized = 11,
};
using enum show_window_command;

/// \brief sets visibility of a window
inline bool show_window(hwnd hw, show_window_command Cmd) noexcept { return ::ShowWindow(hw, int(Cmd)); }
}
