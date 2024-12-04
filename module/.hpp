#pragma once
#if defined(YWLIB) && YWLIB == 2 // for executable
import ywlib;
#else // for coding or compiling module

#include "0.hpp"
#include "1.hpp"
#include "2.hpp"

////////////////////////////////////////////////////////////////////////////////
// WIN32 API

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

#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")

export namespace win {

using hwnd = HWND;

inline bool ok(hwnd hw, const char* Text, const char* Title) noexcept { return ::MessageBoxA(hw, Text, Title, MB_OK) == IDOK; }
inline bool ok(hwnd hw, const wchar_t* Text, const wchar_t* Title) noexcept { return ::MessageBoxW(hw, Text, Title, MB_OK) == IDOK; }

inline bool yes(hwnd hw, const char* Text, const char* Title) noexcept { return ::MessageBoxA(hw, Text, Title, MB_YESNO) == IDYES; }
inline bool yes(hwnd hw, const wchar_t* Text, const wchar_t* Title) noexcept { return ::MessageBoxW(hw, Text, Title, MB_YESNO) == IDYES; }

inline std::string format_error(std::same_as<long> auto hr) noexcept {
  std::string buffer(256, '\0');
  buffer.resize(::FormatMessageA(0x1200, 0, hr, 0, buffer.data(), buffer.size(), 0));
  return buffer;
}

inline double query_performance_frequency() noexcept {
  LARGE_INTEGER li;
  return ::QueryPerformanceFrequency(&li), double(li.QuadPart);
}

inline void query_performance_counter(long long& i) noexcept { ::QueryPerformanceCounter((LARGE_INTEGER*)&i); }

using hinstance = HINSTANCE;

inline hinstance get_module_handle(const char* ModuleName) noexcept { return ::GetModuleHandleA(ModuleName); }
inline hinstance get_module_handle(const wchar_t* ModuleName) noexcept { return ::GetModuleHandleW(ModuleName); }

inline std::vector<std::wstring> get_command_line() noexcept {
  int argc;
  auto a = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
  std::vector<std::wstring> r(argc);
  for (; 0 < argc--;) r[argc] = a[argc];
  ::LocalFree(a);
  return r;
}

inline std::wstring get_username() {
  std::wstring r(256, L'\0');
  unsigned long n;
  ::GetUserNameW(r.data(), &n);
  r.resize(n);
  return r;
}

using hicon = HICON;

inline hicon create_icon_from_file(const wchar_t* FileName) {
  auto bm = (HBITMAP)::LoadImageW(nullptr, FileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
  if (!bm) throw std::runtime_error("failed to load image");
  ICONINFO ii{.fIcon = true, .hbmMask = bm, .hbmColor = bm};
  auto hi = ::CreateIconIndirect(&ii);
  ::DeleteObject(bm);
  return hi;
}

inline bool destroy_icon(hicon hi) { return ::DestroyIcon(hi); }

inline hicon load_icon(hinstance hi, const char* IconName) noexcept { return ::LoadIconA(hi, IconName); }
inline hicon load_icon(hinstance hi, const wchar_t* IconName) noexcept { return ::LoadIconW(hi, IconName); }

using hcursor = HCURSOR;

inline hcursor create_cursor_from_file(const char* FileName) noexcept { return ::LoadCursorFromFileA(FileName); }
inline hcursor create_cursor_from_file(const wchar_t* FileName) noexcept { return ::LoadCursorFromFileW(FileName); }

inline bool destroy_cursor(hcursor hc) noexcept { return ::DestroyCursor(hc); }

inline hcursor load_cursor(hinstance hi, const char* CursorName) noexcept { return ::LoadCursorA(hi, CursorName); }

using hbrush = HBRUSH;

inline hbrush create_solid_brush(const float* RGBA) noexcept {
  static const auto f255 = _mm_set1_ps(255.0f);
  static const auto shuffle = _mm_set1_epi32(0xc0080400);
  auto a = _mm_loadu_ps(RGBA);
  auto b = _mm_cvtps_epi32(_mm_mul_ps(a, f255));
  b = _mm_shuffle_epi8(b, shuffle);
  return ::CreateSolidBrush(unsigned(_mm_cvtsi128_si32(b)));
}

inline bool destroy_brush(hbrush hb) noexcept { return ::DeleteObject(hb); }

enum class message : unsigned {
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
using enum message;
constexpr bool operator==(message a, unsigned b) noexcept { return unsigned(a) == b; }
constexpr bool operator==(unsigned a, message b) noexcept { return a == unsigned(b); }
constexpr auto operator<=>(message a, unsigned b) noexcept { return unsigned(a) <=> b; }
constexpr auto operator<=>(unsigned a, message b) noexcept { return a <=> unsigned(b); }

using msg = MSG;
using wndproc = long long(CALLBACK*)(hwnd, unsigned, unsigned long long, long long);
long long CALLBACK default_procedure(hwnd hw, unsigned msg, unsigned long long wp, long long lp) noexcept { return ::DefWindowProcW(hw, msg, wp, lp); }

enum class class_style : unsigned {
  cs_vredraw = CS_VREDRAW,
  cs_hredraw = CS_HREDRAW,
  cs_dblclks = CS_DBLCLKS,
  cs_owndc = CS_OWNDC,
  cs_classdc = CS_CLASSDC,
  cs_parentdc = CS_PARENTDC,
  cs_noclose = CS_NOCLOSE,
  cs_savebits = CS_SAVEBITS,
  cs_bytealignclient = CS_BYTEALIGNCLIENT,
  cs_bytealignwindow = CS_BYTEALIGNWINDOW,
  cs_globalclass = CS_GLOBALCLASS,
  cs_ime = CS_IME,
  cs_drophshadow = CS_DROPSHADOW,
};
using enum class_style;
constexpr class_style operator|(class_style a, class_style b) noexcept { return class_style(unsigned(a) | unsigned(b)); }
constexpr class_style operator&(class_style a, class_style b) noexcept { return class_style(unsigned(a) & unsigned(b)); }
constexpr class_style operator^(class_style a, class_style b) noexcept { return class_style(unsigned(a) ^ unsigned(b)); }
constexpr class_style operator~(class_style a) noexcept { return class_style(~unsigned(a)); }

inline bool register_class(class_style Style, wndproc Proc, hinstance Instance, hicon Icon, hcursor Cursor, hbrush Background, const char* Name) noexcept {
  WNDCLASSEXA wc{};
  wc.cbSize = sizeof(wc), wc.style = unsigned(Style), wc.lpfnWndProc = Proc, wc.hInstance = Instance;
  wc.hIcon = Icon, wc.hCursor = Cursor, wc.hbrBackground = Background, wc.lpszClassName = Name;
  return ::RegisterClassExA(&wc);
}

inline bool register_class(class_style Style, wndproc Proc, hinstance Instance, hicon Icon, hcursor Cursor, hbrush Background, const wchar_t* Name) noexcept {
  WNDCLASSEXW wc{};
  wc.cbSize = sizeof(wc), wc.style = unsigned(Style), wc.lpfnWndProc = Proc, wc.hInstance = Instance;
  wc.hIcon = Icon, wc.hCursor = Cursor, wc.hbrBackground = Background, wc.lpszClassName = Name;
  return ::RegisterClassExW(&wc);
}

inline bool unregister_class(const char* Name, hinstance Instance) noexcept { return ::UnregisterClassA(Name, Instance); }
inline bool unregister_class(const wchar_t* Name, hinstance Instance) noexcept { return ::UnregisterClassW(Name, Instance); }

namespace control {
inline constexpr auto animation = WC_COMBOBOXEXA;
inline constexpr auto button = WC_BUTTONA;
inline constexpr auto combobox = WC_COMBOBOXEXA;
inline constexpr auto datetime = DATETIMEPICK_CLASSA;
inline constexpr auto edit = WC_EDITA;
inline constexpr auto header = WC_HEADERA;
inline constexpr auto hotkey = HOTKEY_CLASSA;
inline constexpr auto ipaddress = WC_IPADDRESSA;
inline constexpr auto listbox = WC_LISTBOXA;
inline constexpr auto listview = WC_LISTVIEWA;
inline constexpr auto calendar = MONTHCAL_CLASSA;
inline constexpr auto pager = WC_PAGESCROLLERA;
inline constexpr auto progress = PROGRESS_CLASSA;
inline constexpr auto rebar = REBARCLASSNAMEA;
inline constexpr auto scrollbar = WC_SCROLLBARA;
inline constexpr auto static_ = WC_STATICA;
inline constexpr auto statusbar = STATUSCLASSNAMEA;
inline constexpr auto tab = WC_TABCONTROLA;
inline constexpr auto toolbar = TOOLBARCLASSNAMEA;
inline constexpr auto tooltip = TOOLTIPS_CLASSA;
inline constexpr auto trackbar = TRACKBAR_CLASSA;
inline constexpr auto treeview = WC_TREEVIEWA;
inline constexpr auto updown = UPDOWN_CLASSA;
}

enum class virtual_key : unsigned {
  vk_lbutton = VK_LBUTTON,
  vk_rbutton = VK_RBUTTON,
  vk_cancel = VK_CANCEL,
  vk_mbutton = VK_MBUTTON,
  vk_xbutton1 = VK_XBUTTON1,
  vk_xbutton2 = VK_XBUTTON2,
  vk_back = VK_BACK,
  vk_tab = VK_TAB,
  vk_clear = VK_CLEAR,
  vk_return = VK_RETURN,
  vk_shift = VK_SHIFT,
  vk_control = VK_CONTROL,
  vk_menu = VK_MENU,
  vk_pause = VK_PAUSE,
  vk_capital = VK_CAPITAL,
  vk_kana = VK_KANA,
  vk_hangul = VK_HANGUL,
  vk_junja = VK_JUNJA,
  vk_final = VK_FINAL,
  vk_hanja = VK_HANJA,
  vk_kanji = VK_KANJI,
  vk_escape = VK_ESCAPE,
  vk_convert = VK_CONVERT,
  vk_nonconvert = VK_NONCONVERT,
  vk_accept = VK_ACCEPT,
  vk_modechange = VK_MODECHANGE,
  vk_space = VK_SPACE,
  vk_prior = VK_PRIOR,
  vk_next = VK_NEXT,
  vk_end = VK_END,
  vk_home = VK_HOME,
  vk_left = VK_LEFT,
  vk_up = VK_UP,
  vk_right = VK_RIGHT,
  vk_down = VK_DOWN,
  vk_select = VK_SELECT,
  vk_print = VK_PRINT,
  vk_execute = VK_EXECUTE,
  vk_snapshot = VK_SNAPSHOT,
  vk_insert = VK_INSERT,
  vk_delete = VK_DELETE,
  vk_help = VK_HELP,
  vk_0 = '0',
  vk_1 = '1',
  vk_2 = '2',
  vk_3 = '3',
  vk_4 = '4',
  vk_5 = '5',
  vk_6 = '6',
  vk_7 = '7',
  vk_8 = '8',
  vk_9 = '9',
  vk_a = 'A',
  vk_b = 'B',
  vk_c = 'C',
  vk_d = 'D',
  vk_e = 'E',
  vk_f = 'F',
  vk_g = 'G',
  vk_h = 'H',
  vk_i = 'I',
  vk_j = 'J',
  vk_k = 'K',
  vk_l = 'L',
  vk_m = 'M',
  vk_n = 'N',
  vk_o = 'O',
  vk_p = 'P',
  vk_q = 'Q',
  vk_r = 'R',
  vk_s = 'S',
  vk_t = 'T',
  vk_u = 'U',
  vk_v = 'V',
  vk_w = 'W',
  vk_x = 'X',
  vk_y = 'Y',
  vk_z = 'Z',
  vk_lwin = VK_LWIN,
  vk_rwin = VK_RWIN,
  vk_apps = VK_APPS,
  vk_sleep = VK_SLEEP,
  vk_numpad0 = VK_NUMPAD0,
  vk_numpad1 = VK_NUMPAD1,
  vk_numpad2 = VK_NUMPAD2,
  vk_numpad3 = VK_NUMPAD3,
  vk_numpad4 = VK_NUMPAD4,
  vk_numpad5 = VK_NUMPAD5,
  vk_numpad6 = VK_NUMPAD6,
  vk_numpad7 = VK_NUMPAD7,
  vk_numpad8 = VK_NUMPAD8,
  vk_numpad9 = VK_NUMPAD9,
  vk_multiply = VK_MULTIPLY,
  vk_add = VK_ADD,
  vk_separator = VK_SEPARATOR,
  vk_subtract = VK_SUBTRACT,
  vk_decimal = VK_DECIMAL,
  vk_divide = VK_DIVIDE,
  vk_f1 = VK_F1,
  vk_f2 = VK_F2,
  vk_f3 = VK_F3,
  vk_f4 = VK_F4,
  vk_f5 = VK_F5,
  vk_f6 = VK_F6,
  vk_f7 = VK_F7,
  vk_f8 = VK_F8,
  vk_f9 = VK_F9,
  vk_f10 = VK_F10,
  vk_f11 = VK_F11,
  vk_f12 = VK_F12,
  vk_f13 = VK_F13,
  vk_f14 = VK_F14,
  vk_f15 = VK_F15,
  vk_f16 = VK_F16,
  vk_f17 = VK_F17,
  vk_f18 = VK_F18,
  vk_f19 = VK_F19,
  vk_f20 = VK_F20,
  vk_f21 = VK_F21,
  vk_f22 = VK_F22,
  vk_f23 = VK_F23,
  vk_f24 = VK_F24,
  vk_numlock = VK_NUMLOCK,
  vk_scroll = VK_SCROLL,
  vk_oem_nec_equal = VK_OEM_NEC_EQUAL,
  vk_oem_fj_jisho = VK_OEM_FJ_JISHO,
  vk_oem_fj_masshou = VK_OEM_FJ_MASSHOU,
  vk_oem_fj_touroku = VK_OEM_FJ_TOUROKU,
  vk_oem_fj_loya = VK_OEM_FJ_LOYA,
  vk_oem_fj_roya = VK_OEM_FJ_ROYA,
  vk_lshift = VK_LSHIFT,
  vk_rshift = VK_RSHIFT,
  vk_lcontrol = VK_LCONTROL,
  vk_rcontrol = VK_RCONTROL,
  vk_lmenu = VK_LMENU,
  vk_rmenu = VK_RMENU,
  vk_browser_back = VK_BROWSER_BACK,
  vk_browser_forward = VK_BROWSER_FORWARD,
  vk_browser_refresh = VK_BROWSER_REFRESH,
  vk_browser_stop = VK_BROWSER_STOP,
  vk_browser_search = VK_BROWSER_SEARCH,
  vk_browser_favorites = VK_BROWSER_FAVORITES,
  vk_browser_home = VK_BROWSER_HOME,
  vk_volume_mute = VK_VOLUME_MUTE,
  vk_volume_down = VK_VOLUME_DOWN,
  vk_volume_up = VK_VOLUME_UP,
  vk_media_next_track = VK_MEDIA_NEXT_TRACK,
  vk_media_prev_track = VK_MEDIA_PREV_TRACK,
  vk_media_stop = VK_MEDIA_STOP,
  vk_media_play_pause = VK_MEDIA_PLAY_PAUSE,
  vk_launch_mail = VK_LAUNCH_MAIL,
  vk_launch_media_select = VK_LAUNCH_MEDIA_SELECT,
  vk_launch_app1 = VK_LAUNCH_APP1,
  vk_launch_app2 = VK_LAUNCH_APP2,
  vk_oem_1 = VK_OEM_1,
  vk_oem_plus = VK_OEM_PLUS,
  vk_oem_comma = VK_OEM_COMMA,
  vk_oem_minus = VK_OEM_MINUS,
  vk_oem_period = VK_OEM_PERIOD,
  vk_oem_2 = VK_OEM_2,
  vk_oem_3 = VK_OEM_3,
  vk_oem_4 = VK_OEM_4,
  vk_oem_5 = VK_OEM_5,
  vk_oem_6 = VK_OEM_6,
  vk_oem_7 = VK_OEM_7,
  vk_oem_8 = VK_OEM_8,
  vk_oem_ax = VK_OEM_AX,
  vk_oem_102 = VK_OEM_102,
  vk_ico_help = VK_ICO_HELP,
  vk_ico_00 = VK_ICO_00,
  vk_processkey = VK_PROCESSKEY,
  vk_ico_clear = VK_ICO_CLEAR,
  vk_packet = VK_PACKET,
  vk_oem_reset = VK_OEM_RESET,
  vk_oem_jump = VK_OEM_JUMP,
  vk_oem_pa1 = VK_OEM_PA1,
  vk_oem_pa2 = VK_OEM_PA2,
  vk_oem_pa3 = VK_OEM_PA3,
  vk_oem_wsctrl = VK_OEM_WSCTRL,
  vk_oem_cusel = VK_OEM_CUSEL,
  vk_oem_attn = VK_OEM_ATTN,
  vk_oem_finish = VK_OEM_FINISH,
  vk_oem_copy = VK_OEM_COPY,
  vk_oem_auto = VK_OEM_AUTO,
  vk_oem_enlw = VK_OEM_ENLW,
  vk_oem_backtab = VK_OEM_BACKTAB,
  vk_attn = VK_ATTN,
  vk_crsel = VK_CRSEL,
  vk_exsel = VK_EXSEL,
  vk_ereof = VK_EREOF,
  vk_play = VK_PLAY,
  vk_zoom = VK_ZOOM,
  vk_noname = VK_NONAME,
  vk_pa1 = VK_PA1,
  vk_oem_clear = VK_OEM_CLEAR,
};
using enum virtual_key;

enum class window_style : unsigned long long {
  ws_overlapped = WS_OVERLAPPED,
  ws_popup = WS_POPUP,
  ws_child = WS_CHILD,
  ws_minimize = WS_MINIMIZE,
  ws_visible = WS_VISIBLE,
  ws_disabled = WS_DISABLED,
  ws_clipsiblings = WS_CLIPSIBLINGS,
  ws_clipchildren = WS_CLIPCHILDREN,
  ws_maximize = WS_MAXIMIZE,
  ws_caption = WS_CAPTION,
  ws_border = WS_BORDER,
  ws_dlgframe = WS_DLGFRAME,
  ws_vscroll = WS_VSCROLL,
  ws_hscroll = WS_HSCROLL,
  ws_sysmenu = WS_SYSMENU,
  ws_thickframe = WS_THICKFRAME,
  ws_group = WS_GROUP,
  ws_tabstop = WS_TABSTOP,
  ws_minimizebox = WS_MINIMIZEBOX,
  ws_maximizebox = WS_MAXIMIZEBOX,
  ws_tiled = WS_TILED,
  ws_iconic = WS_ICONIC,
  ws_sizebox = WS_SIZEBOX,
  ws_tiledwindow = WS_TILEDWINDOW,
  ws_overlappedwindow = WS_OVERLAPPEDWINDOW,
  ws_popupwindow = WS_POPUPWINDOW,
  ws_childwindow = WS_CHILDWINDOW,
  ws_ex_dlgmmodalframe = WS_EX_DLGMODALFRAME << 32,
  ws_ex_noparentnotify = WS_EX_NOPARENTNOTIFY << 32,
  ws_ex_topmost = WS_EX_TOPMOST << 32,
  ws_ex_acceptfiles = WS_EX_ACCEPTFILES << 32,
  ws_ex_transparent = WS_EX_TRANSPARENT << 32,
  ws_ex_mdichild = WS_EX_MDICHILD << 32,
  ws_ex_toolwindow = WS_EX_TOOLWINDOW << 32,
  ws_ex_windowedge = WS_EX_WINDOWEDGE << 32,
  ws_ex_clientedge = WS_EX_CLIENTEDGE << 32,
  ws_ex_contexthelp = WS_EX_CONTEXTHELP << 32,
  ws_ex_right = WS_EX_RIGHT << 32,
  ws_ex_left = WS_EX_LEFT << 32,
  ws_ex_rtlreading = WS_EX_RTLREADING << 32,
  ws_ex_ltrreading = WS_EX_LTRREADING << 32,
  ws_ex_leftscrollbar = WS_EX_LEFTSCROLLBAR << 32,
  ws_ex_rightscrollbar = WS_EX_RIGHTSCROLLBAR << 32,
  ws_ex_controlparent = WS_EX_CONTROLPARENT << 32,
  ws_ex_staticedge = WS_EX_STATICEDGE << 32,
  ws_ex_appwindow = WS_EX_APPWINDOW << 32,
  ws_ex_overlappedwindow = WS_EX_OVERLAPPEDWINDOW << 32,
  es_left = ES_LEFT,
  es_center = ES_CENTER,
  es_right = ES_RIGHT,
  es_multiline = ES_MULTILINE,
  es_uppercase = ES_UPPERCASE,
  es_lowercase = ES_LOWERCASE,
  es_password = ES_PASSWORD,
  es_autohscroll = ES_AUTOHSCROLL,
  es_autovscroll = ES_AUTOVSCROLL,
  es_nohidesel = ES_NOHIDESEL,
  es_oemconvert = ES_OEMCONVERT,
  es_readonly = ES_READONLY,
  es_wantreturn = ES_WANTRETURN,
  es_number = ES_NUMBER,
  bs_pushbutton = BS_PUSHBUTTON,
  bs_defpushbutton = BS_DEFPUSHBUTTON,
  bs_checkbox = BS_CHECKBOX,
  bs_autocheckbox = BS_AUTOCHECKBOX,
  bs_radiobutton = BS_RADIOBUTTON,
  bs_3state = BS_3STATE,
  bs_auto3state = BS_AUTO3STATE,
  bs_groupbox = BS_GROUPBOX,
  bs_userbutton = BS_USERBUTTON,
  bs_autoradiobutton = BS_AUTORADIOBUTTON,
  bs_pushbox = BS_PUSHBOX,
  bs_ownerdraw = BS_OWNERDRAW,
  bs_typemask = BS_TYPEMASK,
  bs_lefttext = BS_LEFTTEXT,
  bs_text = BS_TEXT,
  bs_icon = BS_ICON,
  bs_bitmap = BS_BITMAP,
  bs_left = BS_LEFT,
  bs_right = BS_RIGHT,
  bs_center = BS_CENTER,
  bs_top = BS_TOP,
  bs_bottom = BS_BOTTOM,
  bs_vcenter = BS_VCENTER,
  bs_pushlike = BS_PUSHLIKE,
  bs_multiline = BS_MULTILINE,
  bs_notify = BS_NOTIFY,
  bs_flat = BS_FLAT,
  bs_rightbutton = BS_RIGHTBUTTON,
  ss_left = SS_LEFT,
  ss_center = SS_CENTER,
  ss_right = SS_RIGHT,
  ss_icon = SS_ICON,
  ss_blackrect = SS_BLACKRECT,
  ss_grayrect = SS_GRAYRECT,
  ss_whiterect = SS_WHITERECT,
  ss_blackframe = SS_BLACKFRAME,
  ss_grayframe = SS_GRAYFRAME,
  ss_whiteframe = SS_WHITEFRAME,
  ss_useritem = SS_USERITEM,
  ss_simple = SS_SIMPLE,
  ss_leftnowordwrap = SS_LEFTNOWORDWRAP,
  ss_ownerdraw = SS_OWNERDRAW,
  ss_bitmap = SS_BITMAP,
  ss_enhmetafile = SS_ENHMETAFILE,
  ss_etchedhorz = SS_ETCHEDHORZ,
  ss_etchedvert = SS_ETCHEDVERT,
  ss_etchedframe = SS_ETCHEDFRAME,
  ss_type_mask = SS_TYPEMASK,
  ss_realsizecontrol = SS_REALSIZECONTROL,
  ss_noprefix = SS_NOPREFIX,
  ss_notify = SS_NOTIFY,
  ss_centerimage = SS_CENTERIMAGE,
  ss_rightjust = SS_RIGHTJUST,
  ss_realsizeimage = SS_REALSIZEIMAGE,
  ss_sunken = SS_SUNKEN,
  ss_editcontrol = SS_EDITCONTROL,
  ss_endellipsis = SS_ENDELLIPSIS,
  ss_pathellipsis = SS_PATHELLIPSIS,
  ss_wordellipsis = SS_WORDELLIPSIS,
  ss_ellipsismask = SS_ELLIPSISMASK,
  lbs_notify = LBS_NOTIFY,
  lbs_sort = LBS_SORT,
  lbs_noredo = LBS_NOREDRAW,
  lbs_multiplesel = LBS_MULTIPLESEL,
  lbs_ownerdrawfixed = LBS_OWNERDRAWFIXED,
  lbs_ownerdrawvariable = LBS_OWNERDRAWVARIABLE,
  lbs_hasstrings = LBS_HASSTRINGS,
  lbs_usertabstops = LBS_USETABSTOPS,
  lbs_nointegralheight = LBS_NOINTEGRALHEIGHT,
  lbs_multicolumn = LBS_MULTICOLUMN,
  lbs_wantkeyboardinput = LBS_WANTKEYBOARDINPUT,
  lbs_extendedsel = LBS_EXTENDEDSEL,
  lbs_disablenoscroll = LBS_DISABLENOSCROLL,
  lbs_nodata = LBS_NODATA,
  lbs_nosel = LBS_NOSEL,
  lbs_standard = LBS_STANDARD,
  cbs_simple = CBS_SIMPLE,
  cbs_dropdown = CBS_DROPDOWN,
  cbs_dropdownlist = CBS_DROPDOWNLIST,
  cbs_ownerdrawfixed = CBS_OWNERDRAWFIXED,
  cbs_ownerdrawvariable = CBS_OWNERDRAWVARIABLE,
  cbs_autohscroll = CBS_AUTOHSCROLL,
  cbs_oemconvert = CBS_OEMCONVERT,
  cbs_sort = CBS_SORT,
  cbs_hasstrings = CBS_HASSTRINGS,
  cbs_nointegralheight = CBS_NOINTEGRALHEIGHT,
  cbs_disablenoscroll = CBS_DISABLENOSCROLL,
  cbs_uppercase = CBS_UPPERCASE,
  cbs_lowercase = CBS_LOWERCASE,
  sbs_horz = SBS_HORZ,
  sbs_vert = SBS_VERT,
  sbs_topalign = SBS_TOPALIGN,
  sbs_leftalign = SBS_LEFTALIGN,
  sbs_bottomalign = SBS_BOTTOMALIGN,
  sbs_rightalign = SBS_RIGHTALIGN,
  sbs_sizeboxtopleftalign = SBS_SIZEBOXTOPLEFTALIGN,
  sbs_sizeboxbottomrightalign = SBS_SIZEBOXBOTTOMRIGHTALIGN,
  sbs_sizebox = SBS_SIZEBOX,
  sbs_sizegrip = SBS_SIZEGRIP,
};
constexpr window_style operator|(window_style a, window_style b) noexcept { return window_style(unsigned(a) | unsigned(b)); }
constexpr window_style operator&(window_style a, window_style b) noexcept { return window_style(unsigned(a) & unsigned(b)); }
constexpr window_style operator^(window_style a, window_style b) noexcept { return window_style(unsigned(a) ^ unsigned(b)); }
constexpr window_style operator~(window_style a) noexcept { return window_style(~unsigned(a)); }

inline void translate_message(const msg& m) noexcept { ::TranslateMessage(&m); }
inline void dispatch_message(const msg& m) noexcept { ::DispatchMessageW(&m); }

inline hwnd create_window(window_style Style, const char* ClassName, const char* WindowName, int X, int Y, int Width, int Height, hwnd Parent, hinstance Instance) {
  return ::CreateWindowExA(unsigned(unsigned long long(Style) >> 32), ClassName, WindowName,      //
                           unsigned(unsigned long long(Style) & 0xFFFFFFFF), X, Y, Width, Height, //
                           Parent, nullptr, Instance, nullptr);
}

inline bool get_client_rect(hwnd hw, int* LeftTopRightBottom) noexcept { return ::GetClientRect(hw, reinterpret_cast<RECT*>(LeftTopRightBottom)); }

inline bool set_window_pos(hwnd hw, int X, int Y, int Top, int Right) noexcept { return ::SetWindowPos(hw, nullptr, X, Y, Top, Right, SWP_NOZORDER); }

enum class show_window_command : int {
  sw_hide = SW_HIDE,
  sw_shownormal = SW_SHOWNORMAL,
  sw_showminimized = SW_SHOWMINIMIZED,
  sw_maximize = SW_MAXIMIZE,
  sw_shownoactivate = SW_SHOWNOACTIVATE,
  sw_show = SW_SHOW,
  sw_minimize = SW_MINIMIZE,
  sw_showminnoactive = SW_SHOWMINNOACTIVE,
  sw_showna = SW_SHOWNA,
  sw_restore = SW_RESTORE,
  sw_showdefault = SW_SHOWDEFAULT,
  sw_forceminimize = SW_FORCEMINIMIZE,
};
using enum show_window_command;

inline bool show_window(hwnd hw, show_window_command Command) { return ::ShowWindow(hw, int(Command)); }

inline bool focus_window(hwnd hw) { return ::SetFocus(hw); }

inline bool destroy_window(hwnd hw) { return ::DestroyWindow(hw); }

template<typename Ct> std::basic_string<Ct> get_window_text(hwnd hw) {
  if constexpr (sizeof(Ct) == 1) {
    std::basic_string<Ct> r(::GetWindowTextLengthA(hw), Ct{});
    ::GetWindowTextA(hw, (char*)r.data(), unsigned(r.size() + 1));
    return r;
  } else if constexpr (sizeof(Ct) == 2) {
    std::basic_string<Ct> r(::GetWindowTextLengthW(hw), Ct{});
    ::GetWindowTextW(hw, (wchar_t*)r.data(), unsigned(r.size() + 1));
    return r;
  } else if constexpr (sizeof(Ct) == 4) {
    std::basic_string<wchar_t> s(::GetWindowTextLengthW(hw), wchar_t{});
    std::basic_string<Ct> r(s.size(), Ct{});
    ::GetWindowTextW(hw, s.data(), unsigned(s.size() + 1));
    auto out = r.begin();
    for (auto in = s.begin(); in != s.end();) { // convert UTF-16 to UTF-32
      if (*in < 0x10000) *out++ = *in++;
      else *out++ = (*in++ - 0xD800) << 10 | (*in++ - 0xDC00) | 0x10000;
    }
    r.resize(out - r.begin());
    return r;
  }
}

inline bool set_window_text(hwnd hw, const char* Text) { return ::SetWindowTextA(hw, Text); }
inline bool set_window_text(hwnd hw, const wchar_t* Text) { return ::SetWindowTextW(hw, Text); }

} // namespace win

#endif
