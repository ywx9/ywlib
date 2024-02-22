/// @file ywlib/main.h

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Shlwapi.h>
#include <Windows.h>
#include <commdlg.h>
#include <d2d1_3.h>
#include <d3d11_3.h>
#include <d3dcompiler.h>
#include <dwrite_3.h>
#include <dxgi1_6.h>
#include <shellapi.h>
#include <wincodec.h>
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "uuid.lib")
#undef max
#undef min

#include "xv.h"

namespace yw { // clang-format off

#ifdef ywlib_debug
#define _DEBUG
#define ywlib_enable_console
#define ywlib_assert(Bool, Str) (Bool ? void() : (void)(std::cerr << Str << std::endl, throw yw::except(Str)))
inline constexpr nodebug = false;
#else
#define ywlib_assert(Bool, Str) (void())
inline constexpr bool nodebug = true;
#endif

/// comptr structure
template<typename Com> class comptr {
  static_assert(requires (Com& a, IUnknown* b) {
    { a.QueryInterface(__uuidof(IUnknown), (void**)&b) } -> yw::same_as<HRESULT>;
    { a.QueryInterface(&b) } -> same_as<HRESULT>; { a.Release() } -> same_as<ULONG>; { a.AddRef() } -> same_as<ULONG>; });
  template<typename U> friend class comptr;
  Com* _ptr = nullptr;
  void _addref() const noexcept { _ptr ? void(_ptr->AddRef()) : void(); }
  void _release() noexcept { _ptr ? void(exchange(_ptr, nullptr)->Release()) : void(); }
  void _copy(Com* a) noexcept { _ptr == a ? void() : (_release(), _ptr = a, _addref()); }
  void _move(auto& a) noexcept { _ptr == a._ptr ? void() : void((_release(), _ptr = a._ptr, a._ptr = nullptr)); }
public:
  using pointer = Com*;
  class reference : public Com {
    void operator&() const = delete;
    ULONG __stdcall AddRef() { return Com::AddRef(); }
    ULONG __stdcall Release() { return Com::Release(); } };
  ~comptr() noexcept { _release(); }
  comptr() noexcept = default;
  comptr(comptr&& a) noexcept : _ptr{a._ptr} { a._ptr = nullptr; }
  comptr(const comptr& a) noexcept : _ptr{a._ptr} { _addref(); }
  comptr(Com*&& a) noexcept : _ptr(a) {}
  template<typename U> comptr(comptr<U>&& a) noexcept : _ptr{a._ptr} { a._ptr = nullptr; }
  template<typename U> comptr(const comptr<U>& a) noexcept : _ptr{a._ptr} { _addref(); }
  comptr& operator=(const comptr& a) noexcept { return _copy(a._ptr), *this; }
  template<typename U> comptr& operator=(const comptr<U>& a) noexcept { return _copy(a._ptr), *this; }
  template<typename U> comptr& operator=(comptr<U>&& a) noexcept { return _move(a), *this; }
  template<typename U> bool operator==(const comptr<U>& a) noexcept { return _ptr == a._ptr; }
  explicit operator bool() const noexcept { return _ptr; }
  operator reference*() const noexcept { return static_cast<reference*>(_ptr); }
  reference* operator->() const noexcept { return static_cast<reference*>(_ptr); }
  void reset() noexcept { _release(); }
  Com* get() const noexcept { return _ptr; }
  Com** addressof(source S = {}) & { if (_ptr != nullptr) throw except("You must reset comptr", mv(S)); return &_ptr; }
  Com* const* addressof() const noexcept { return &_ptr; }
  void swap(comptr& a) noexcept { std::ranges::swap(_ptr, a._ptr); }
  template<typename U> HRESULT as(comptr<U>& a) const noexcept { return _ptr->QueryInterface(IID_PPV_ARGS(a.addressof())); }
};

/// stopwatch class
class stopwatch {
  mutable LARGE_INTEGER _li;
  fat8 _freq;
  int8 _last;
public:
  stopwatch() noexcept : _li{}, _freq{}, _last{} {
    QueryPerformanceFrequency(&_li), _freq = static_cast<fat8>(_li.QuadPart), QueryPerformanceCounter(&_li), _last = _li.QuadPart; }
  fat8 operator()() const noexcept { return read(); }
  fat8 read() const noexcept { return QueryPerformanceCounter(&_li), (_li.QuadPart - _last) / _freq; }
  fat8 push() noexcept { return QueryPerformanceCounter(&_li), (_li.QuadPart - exchange(_last, _li.QuadPart)) / _freq; }
  void start() noexcept { QueryPerformanceCounter(&_li), _last = _li.QuadPart; }
};

namespace main {

namespace system {
inline str2 username{};
inline array<str2> args{};
inline stopwatch timer{};
inline HWND hwnd{};
inline HFONT hfont{};
inline HINSTANCE hinstance{};
inline rect mouse{};
inline bool hover{};
inline fat8 spf{}, fps{};
inline comptr<ID3D11Device1> d3d_device{};
inline comptr<ID3D11DeviceContext1> d3d_context{};
inline comptr<IDXGISwapChain1> swap_chain{};
inline comptr<ID2D1Device5> d2d_device{};
inline comptr<ID2D1DeviceContext5> d2d_context{};
inline comptr<ID2D1Factory6> d2d_factory{};
inline comptr<IDWriteFactory> dw_factory{};
inline comptr<IWICImagingFactory2> wic_factory{};
LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);
inline void initialize();
}

inline const str2& username = system::username;
inline const array<str2>& args = system::args;
inline const stopwatch& timer = system::timer;
inline const fat8& spf = system::spf;
inline const fat8& fps = system::fps;
inline const bool& hover = system::hover;
inline const HWND& hwnd = system::hwnd;
inline const HINSTANCE& hinstance = system::hinstance;
inline const comptr<ID3D11Device1>& d3d_device = system::d3d_device;
inline const comptr<ID3D11DeviceContext1>& d3d_context = system::d3d_context;
inline const comptr<IDXGISwapChain1>& swap_chain = system::swap_chain;
inline const comptr<ID2D1Device5>& d2d_device = system::d2d_device;
inline const comptr<ID2D1DeviceContext5>& d2d_context = system::d2d_context;
inline const comptr<ID2D1Factory6>& d2d_factory = system::d2d_factory;
inline const comptr<IDWriteFactory>& dw_factory = system::dw_factory;
inline const comptr<IWICImagingFactory2>& wic_factory = system::wic_factory;
inline void resize(natt Width, natt Height);
}

inline void main::system::initialize() {
  static constexpr cat2 class_name[] = L"ywlib";
  [&](DWORD d) { ::GetUserNameW(nullptr, &d); username.resize(d - 1); ::GetUserNameW(username.data(), &d); }({});
  hinstance = GetModuleHandleW(nullptr);
  [&](WNDCLASSEXW wc) { wc.hCursor = LoadCursorW(nullptr, IDC_ARROW); wc.lpszClassName = class_name;
                        tiff(RegisterClassExW(&wc)); }({sizeof(WNDCLASSEXW), CS_OWNDC, wndproc, 0, 0, hinstance});
  tiff(hwnd = CreateWindowExW(WS_EX_ACCEPTFILES, class_name, class_name, WS_OVERLAPPED,
                              704, 284, 512, 512, nullptr, nullptr, hinstance, nullptr));
  tiff(system::hfont = CreateFontW(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
                                   CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_ROMAN, L"Yu Gothic UI"));
  [&](int c) { auto args = CommandLineToArgvW(GetCommandLineW(), &c);
               for (system::args.resize(c); 0 <= --c;) system::args[c] = args[c]; }(0);
}

/// opens a message box with OK button and returns `true`
inline constexpr overload ok{
  [](const stv1& Text, const stv1& Caption = "OK?") noexcept { return MessageBoxA(main::hwnd, Text.data(), Caption.data(), MB_OK | MB_ICONEXCLAMATION), true; },
  [](const stv2& Text, const stv2& Caption = L"OK?") noexcept { return MessageBoxW(main::hwnd, Text.data(), Caption.data(), MB_OK | MB_ICONEXCLAMATION), true; }};

/// opens a message box with Yes/No buttons and returns `true` if Yes is selected
inline constexpr overload yes{
  [](const stv1& Text, const stv1& Caption = "Yes?") noexcept { return MessageBoxA(main::hwnd, Text.data(), Caption.data(), MB_YESNO | MB_ICONQUESTION) == IDYES; },
  [](const stv2& Text, const stv2& Caption = L"Yes?") noexcept { return MessageBoxW(main::hwnd, Text.data(), Caption.data(), MB_YESNO | MB_ICONQUESTION) == IDYES; }};

/// throws a exception if failed
inline constexpr auto tiff = overload{
  [](HRESULT B, source S = {}) {
    if (B < 0) { cat1 buf[128];
      FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, B, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 128, nullptr);
      throw except(buf, mv(S)); } },
  [](const auto& B, source S = {}) requires requires { { !B } -> convertible_to<bool>; } { if (!B) throw except("FAILED", mv(S)); }};

/// displays the exception message
inline constexpr overload disp = {
  [](const std::exception& E) noexcept { if constexpr (nodebug) ok(E.what(), "Error"); else std::cerr << E.what() << std::endl; },
  [](const std::exception& E, none Relay) { if constexpr (nodebug) ok(E.what(), "Error"), throw E; else std::cerr << E.what() << std::endl, throw E; }};

/// creates a file open dialog and returns the selected file
inline constexpr auto open_file = [](const path& InitialDir = {}) noexcept -> path {
  try { cat2 buf[260]{0};
        OPENFILENAMEW ofn{DWORD(sizeof(ofn)), main::hwnd, 0, L"All Files (*.*)\0*.*\0", 0, 0, 1, buf, nat4(extent<decltype(buf)>)};
        ofn.lpstrInitialDir = InitialDir.empty() ? nullptr : InitialDir.c_str(), ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        if (GetOpenFileNameW(&ofn) == TRUE) return path(ofn.lpstrFile);
  } catch (const std::exception& E) { disp(E); } return path{}; };

/// creates a file save dialog and returns the selected file
inline constexpr auto save_file = [](const path& InitialDir = {}, const path& InitialFileName = {}) -> path {
  try { cat2 buf[260]{0};
        if (!InitialFileName.empty()) std::ranges::copy(InitialFileName.filename().wstring(), buf);
        OPENFILENAMEW ofn{DWORD(sizeof(ofn)), main::hwnd, 0, L"All Files (*.*)\0*.*\0", 0, 0, 1, buf, nat4(extent<decltype(buf)>)};
        ofn.lpstrInitialDir = InitialDir.empty() ? nullptr : InitialDir.c_str(), ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        if (GetSaveFileNameW(&ofn) == TRUE) return path(ofn.lpstrFile);
  } catch (const std::exception& E) { disp(E); } return path{}; };

namespace key {
/// class for handling key states
class key {
  friend LRESULT CALLBACK ::yw::main::wndproc(HWND, UINT, WPARAM, LPARAM);
  fat8 pushed_time{};
public:
  explicit operator bool() const noexcept { return pushed_time != 0; }
  /// callback function that is called when the key is pressed
  void (*down)() = nullptr;
  /// callback function that is called when the key is released
  void (*up)(fat8 hold_time) = nullptr;
};

inline array<key, 256> keys{};
inline key &lbutton = keys[VK_LBUTTON], &rbutton = keys[VK_RBUTTON], &mbutton = keys[VK_MBUTTON];
inline key &xbutton1 = keys[VK_XBUTTON1], &xbutton2 = keys[VK_XBUTTON2], &win = keys[VK_LWIN], &app = keys[VK_APPS];
inline key &back = keys[VK_BACK], &tab = keys[VK_TAB], &enter = keys[VK_RETURN], &escape = keys[VK_ESCAPE];
inline key &shift = keys[VK_SHIFT], &control = keys[VK_CONTROL], &alt = keys[VK_MENU], &space = keys[VK_SPACE];
inline key &pageup = keys[VK_PRIOR], &pagedown = keys[VK_NEXT], &end = keys[VK_END], &home = keys[VK_HOME];
inline key &left = keys[VK_LEFT], &up = keys[VK_UP], &right = keys[VK_RIGHT], &down = keys[VK_DOWN];
inline key &screenshot = keys[VK_SNAPSHOT], &insert = keys[VK_INSERT], &delete_ = keys[VK_DELETE];
inline key &n0 = keys['0'], &n1 = keys['1'], &n2 = keys['2'], &n3 = keys['3'], &n4 = keys['4'];
inline key &n5 = keys['5'], &n6 = keys['6'], &n7 = keys['7'], &n8 = keys['8'], &n9 = keys['9'];
inline key &a = keys['A'], &b = keys['B'], &c = keys['C'], &d = keys['D'], &e = keys['E'], &f = keys['F'], &g = keys['G'], &h = keys['H'], &i = keys['I'];
inline key &j = keys['J'], &k = keys['K'], &l = keys['L'], &m = keys['M'], &n = keys['N'], &o = keys['O'], &p = keys['P'], &q = keys['Q'], &r = keys['R'];
inline key &s = keys['S'], &t = keys['T'], &u = keys['U'], &v = keys['V'], &w = keys['W'], &x = keys['X'], &y = keys['Y'], &z = keys['Z'];
inline key &np_0 = keys[VK_NUMPAD0], &np_1 = keys[VK_NUMPAD1], &np_2 = keys[VK_NUMPAD2], &np_3 = keys[VK_NUMPAD3], &np_4 = keys[VK_NUMPAD4];
inline key &np_5 = keys[VK_NUMPAD5], &np_6 = keys[VK_NUMPAD6], &np_7 = keys[VK_NUMPAD7], &np_8 = keys[VK_NUMPAD8], &np_9 = keys[VK_NUMPAD9];
inline key &f1 = keys[VK_F1], &f2 = keys[VK_F2], &f3 = keys[VK_F3], &f4 = keys[VK_F4], &f5 = keys[VK_F5], &f6 = keys[VK_F6];
inline key &f7 = keys[VK_F7], &f8 = keys[VK_F8], &f9 = keys[VK_F9], &f10 = keys[VK_F10], &f11 = keys[VK_F11], &f12 = keys[VK_F12];
inline key &minus = keys[VK_OEM_MINUS], &plus = keys[VK_OEM_PLUS], &comma = keys[VK_OEM_COMMA], &period = keys[VK_OEM_PERIOD];
inline key &oem00 = keys[VK_OEM_7], &oem01 = keys[VK_OEM_5], &oem10 = keys[VK_OEM_3], &oem11 = keys[VK_OEM_4];
inline key &oem20 = keys[VK_OEM_1], &oem21 = keys[VK_OEM_6], &oem30 = keys[VK_OEM_2], &oem31 = keys[VK_OEM_102];
}

/// empty structure for handling mouse states
struct mouse {
  /// x-position of the mouse
  inline static const auto& x = main::system::mouse.left;
  /// y-position of the mouse
  inline static const auto& y = main::system::mouse.top;
  /// checks if the mouse is hovering over the window
  inline static const bool& hover = main::hover;
  /// callback function for mouse wheel
  inline static void (*wheeled)(fat4 delta) = nullptr;
  /// callback function for mouse movement
  inline static void (*moved)(int4 dx, int4 dy) = nullptr;
  /// alias of `key::lbutton`
  inline static auto& left = key::lbutton;
  /// alias of `key::rbutton`
  inline static auto& right = key::rbutton;
  /// alias of `key::mbutton`
  inline static auto& middle = key::mbutton;
  /// alias of `key::xbutton1`
  inline static auto& ex1 = key::xbutton1;
  /// alias of `key::xbutton2`
  inline static auto& ex2 = key::xbutton2;
  /// sets the new position of the mouse
  mouse(int4 X, int4 Y) noexcept { SetCursorPos(X, Y); }
};

/// base class for creating a control
class control {
public:
  inline static constexpr natt max_groups = 16;
  static void hide(natt GroupNo, bool Hide = true) {
    ywlib_assert(GroupNo < max_groups, "GroupNo must be less than max_groups"); for (auto& g : groups[GroupNo]) g.second->hide(Hide);}
  const HWND hwnd{};
  const natt group{npos};
  ~control() noexcept {
    if (!hwnd) return;
    DestroyWindow(hwnd), whole_controls.erase(hwnd);
    groups[group + 1].erase(groups[group + 1].begin() + get_index(groups[group + 1], hwnd));  }
  control() noexcept = default;
  control(control&& A) : hwnd(exchange(const_cast<HWND&>(A.hwnd), nullptr)), group(A.group) {
    whole_controls[hwnd].second = this, groups[group + 1][get_index(groups[group + 1], hwnd)].second = this; }
  control& operator=(control&& A) {
    if (auto&g = groups[group + 1]; hwnd) DestroyWindow(hwnd), g.erase(g.begin() + get_index(g, hwnd)), whole_controls.erase(hwnd);
    const_cast<HWND&>(hwnd) = exchange(const_cast<HWND&>(A.hwnd), nullptr), const_cast<natt&>(group) = A.group;
    return (hwnd ? whole_controls[hwnd].second = this, groups[group + 1][get_index(groups[group + 1], hwnd)].second = this : none{}), *this; }
  explicit operator bool() const noexcept { return hwnd; }
  virtual void setfocus() const { ywlib_assert(hwnd, "SetFocus failed"); SetFocus(hwnd); }
  virtual void hide(bool Hide = true) const { ywlib_assert(hwnd, "ShowWindow failed"); ShowWindow(hwnd, Hide ? SW_HIDE : SW_SHOW); }
  /// sets the text of the control
  /// @note `Text` must be null-terminated.
  virtual void text(const stv2& Text) const { ywlib_assert(hwnd, "SetWindowText failed"); SetWindowTextW(hwnd, Text.data()); }
  /// gets the text of the control
  virtual str2 text() const {
    ywlib_assert(hwnd, "GetWindowText failed");
    str2 out(GetWindowTextLengthW(hwnd), {}); GetWindowTextW(hwnd, out.data(), int4(out.size() + 1)); return out; }
protected:
  inline static array<array<list<HWND, control*, bool>>, max_groups + 1> groups;
  inline static std::map<HWND, list<natt, control*>> whole_controls;
  control(const cat2* Class, const stv2& Text, nat4 Style, const rect& Rect, bool Inputable)
    : hwnd(CreateWindowExW(0, Class, Text.data(), WS_VISIBLE | WS_CHILD | WS_GROUP | Style, Rect.left, Rect.top,
                           Rect.width(), Rect.height(), main::hwnd, 0, main::hinstance, this)), group(npos) {
    SendMessageW(hwnd, WM_SETFONT, (WPARAM)main::system::hfont, true); SetWindowLongPtrW(hwnd, GWLP_ID, (LONG_PTR)hwnd);
    if (groups[0].empty()) groups[0].reserve(1024);
    groups[0].push_back(list{hwnd, this, Inputable}), whole_controls[hwnd] = list{npos, this}; }
  control(const cat2* Class, const stv2& Text, nat4 Style, const rect& Rect, const natt GroupNo, bool Inputable)
    : hwnd(CreateWindowExW(0, Class, Text.data(), WS_VISIBLE | WS_CHILD | WS_GROUP | Style, Rect.left, Rect.top,
                           Rect.width(), Rect.height(), main::hwnd, 0, main::hinstance, this)), group(GroupNo) {
    ywlib_assert(GroupNo < max_groups, "Control group index is out of range");
    if (!hfont) hfont = CreateFontW(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
                                    CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_ROMAN, L"Yu Gothic UI");
    SendMessageW(hwnd, WM_SETFONT, (WPARAM)hfont, true), SetWindowLongPtrW(hwnd, GWLP_ID, (LONG_PTR)hwnd);
    if (groups[GroupNo + 1].empty()) groups[GroupNo + 1].reserve(1024);
    groups[GroupNo + 1].push_back(list{hwnd, this, Inputable}), whole_controls[hwnd] = list{GroupNo, this}; }
  static array<list<HWND, control*, bool>>& get_group(const natt GroupNo) {
    ywlib_assert(GroupNo < max_groups, "Control group index is out of range"); return groups[GroupNo + 1]; }
  static natt get_index(const array<list<HWND, control*, bool>>& Group, HWND Handle) {
    for (natt i{}; i < Group.size(); ++i) if (Group[i].first == Handle) return i; return npos; }
  static void focus_on_next(const array<list<HWND, control*, bool>>& Group, natt Now, bool Prev) {
    if (Prev) { for (natt j = Now - 1; j < Now; --j) if (Group[j].third) return Group[j].second->setfocus(), std::cout << j << std::endl, void();
                for (natt j = Group.size() - 1; Now < j; --j) if (Group[j].third) return Group[j].second->setfocus(), std::cout << j << std::endl, void();
    } else {    for (natt j = Now + 1; j < Group.size(); ++j) if (Group[j].third) return Group[j].second->setfocus(), std::cout << j << std::endl, void();
                for (natt j = 0; j < Now; ++j) if (Group[j].third) return Group[j].second->setfocus(), std::cout << j << std::endl, void(); } }
};

/// class for creating a label control
class label : public control {
public:
  label() noexcept = default;
  label(const rect& Rect, const stv2& Text, nat4 StaticStyle = {}) : control(L"STATIC", Text, StaticStyle, Rect, false) {}
  label(const natt GroupNo, const rect& Rect, const stv2& Text, nat4 StaticStyle = {}) : control(L"STATIC", Text, StaticStyle, Rect, GroupNo, false) {}
};

/// class for creating textbox controls
class textbox : public control {
protected:
  inline static WNDPROC defproc = nullptr;
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_CHAR) {
      if (wp == VK_RETURN) if (auto& t = *static_cast<textbox*>(whole_controls[hw].second); t.enter) return t.enter(t), 0;
      else if (wp == VK_TAB) {
        auto& g = get_group(whole_controls[hw].first); auto i = get_index(g, hw); auto& t = *static_cast<textbox*>(g[i].second);
        if (t.tab) t.tab(t); return focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0), 0;
      } else if (wp == VK_ESCAPE) return SetFocus(main::hwnd), 0;
    } else if (msg == WM_SETFOCUS) if (auto& t = *static_cast<textbox*>(whole_controls[hw].second); t.intofocus) return t.intofocus(t), 0;
    else if (msg == WM_KILLFOCUS) if (auto& t = *static_cast<textbox*>(whole_controls[hw].second); t.killfocus) return t.killfocus(t), 0;
    return CallWindowProcW(defproc, hw, msg, wp, lp); }
public:
  void (*enter)(const textbox& This) = nullptr;
  void (*tab)(const textbox& This) = nullptr;
  void (*intofocus)(const textbox& This) = nullptr;
  void (*killfocus)(const textbox& This) = nullptr;
  textbox() noexcept = default;
  textbox(const rect& Rect, const stv2& Init = L"", nat4 EditStyle = {}) : control(L"EDIT", Init, EditStyle, Rect, true) {
    if (!defproc) defproc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); }
  textbox(const natt GroupNo, const rect& Rect, const stv2& Init, nat4 EditStyle = {}) : control(L"EDIT", Init, EditStyle, Rect, GroupNo, true) {
    if (!defproc) defproc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); }
};

/// class for creating a valuebox control
class valuebox : public control {
protected:
  inline static WNDPROC defproc = nullptr;
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_CHAR) {
      if (wp == VK_RETURN) if (auto& t = *static_cast<valuebox*>(whole_controls[hw].second); t.enter) return t.enter(t), 0;
      else if (wp == VK_TAB) {
        auto& g = get_group(whole_controls[hw].first); auto i = get_index(g, hw); auto& t = *static_cast<valuebox*>(g[i].second);
        if (t.tab) t.tab(t); return focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0), 0;
      } else if (wp == VK_ESCAPE) return SetFocus(main::hwnd), 0;
      else if (wp == VK_BACK || wp == VK_DELETE || wp == VK_LEFT || wp == VK_RIGHT) void(0);
      else if (!(wp >= '0' && wp <= '9' || wp == '-' || wp == '.')) return 0;
    } else if (msg == WM_SETFOCUS) if (auto& t = *static_cast<valuebox*>(whole_controls[hw].second); t.intofocus) return t.intofocus(t), 0;
    else if (msg == WM_KILLFOCUS) if (auto& t = *static_cast<valuebox*>(whole_controls[hw].second); t.killfocus) return t.killfocus(t), 0;
    return CallWindowProcW(defproc, hw, msg, wp, lp); }
public:
  void (*enter)(const valuebox& This) = nullptr;
  void (*tab)(const valuebox& This) = nullptr;
  void (*intofocus)(const valuebox& This) = nullptr;
  void (*killfocus)(const valuebox& This) = nullptr;
  valuebox() noexcept = default;
  valuebox(const rect& Rect, const arithmetic auto Init, nat4 EditStyle = {}) : control(L"EDIT", vtos<cat2>(Init), EditStyle, Rect, true) {
    if (!defproc) defproc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); }
  valuebox(const natt GroupNo, const rect& Rect, const arithmetic auto Init, nat4 EditStyle = {}) : control(L"EDIT", vtos<cat2>(Init), EditStyle, Rect, GroupNo, true) {
    if (!defproc) defproc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); }
  void value(const arithmetic auto Value) const { SetWindowTextW(hwnd, vtos<cat2>(Value).data()); }
  template<arithmetic T> T value() const { return stov<T>(text()); }
};

/// class for obtaining the button state
template<typename Button, invocable<const Button&> Converter> class button_state {
  const Button* ptr;
  Converter func;
public:
  template<typename F> button_state(const Button* B, F&& Func) noexcept : ptr(B), func(fwd<F>(Func)) {}
  operator invoke_result<Converter, const Button&>() const { return func(*ptr); }
};
template<typename B, typename F> button_state(const B*, F&&) -> button_state<B, F>;

/// class for creating button controls
class button : public control {
  static bool get_state(const button& This) { return bool(defproc(This.hwnd, BM_GETSTATE, 0, 0) & BST_PUSHED); }
protected:
  inline static WNDPROC defproc = nullptr;
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_LBUTTONDOWN)
      if (auto& t = *static_cast<button*>(whole_controls[hw].second); t.enter)
        return [&](auto r) { return t.enter(t), r; }(defproc(hw, msg, wp, lp));
    else if (msg == WM_KEYDOWN) {
      if (wp == VK_RETURN || wp == VK_SPACE) {
        if (auto& t = *static_cast<button*>(whole_controls[hw].second); t.enter) return [&](auto r) { return t.enter(t), r; }(defproc(hw, msg, wp, lp));
      } else if (wp == VK_TAB) {
        auto& g = get_group(whole_controls[hw].first); auto i = get_index(g, hw); return focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0), 0;
      } else if (wp == VK_ESCAPE) return SetFocus(main::hwnd), 0; }
    return CallWindowProcW(defproc, hw, msg, wp, lp); }
public:
  const button_state<button, decltype(&get_state)> state;
  void (*enter)(const button& This) = nullptr;
  button() noexcept : control(), state(this, get_state) {}
  button(button&& A) : control(mv(dynamic_cast<control&>(A))), state(this, get_state), enter(A.enter) {}
  button& operator=(button&& A) { return dynamic_cast<control&>(*this) = mv(dynamic_cast<control&>(A)), enter = A.enter, *this; }
  button(const rect& Rect, const stv2& Text, nat4 ButtonStyle = {}) : control(L"BUTTON", Text, ButtonStyle, Rect, true), state(this, get_state) {
    if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC); SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); }
  button(natt GroupNo, const rect& Rect, const stv2& Text, nat4 ButtonStyle = {}) : control(L"BUTTON", Text, ButtonStyle, Rect, GroupNo, true),
    state(this, get_state) { if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC); SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); }
};

/// class for creating checkbox controls
class checkbox : public control {
  static bool get_state(const checkbox& This) { return bool(defproc(This.hwnd, BM_GETCHECK, 0, 0) & BST_CHECKED); }
protected:
  inline static WNDPROC defproc = nullptr;
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_LBUTTONUP) {
      if (auto& t = *static_cast<checkbox*>(whole_controls[hw].second); t.enter) return [&](auto r) { return t.enter(t), r; }(defproc(hw, msg, wp, lp));
    } else if (msg == WM_KEYUP) {
      if (wp == VK_RETURN || wp == VK_SPACE)
        if (auto& t = *static_cast<checkbox*>(whole_controls[hw].second); t.enter) return [&](auto r) { return t.enter(t), r; }(defproc(hw, msg, VK_SPACE, lp));
    } else if (msg == WM_KEYDOWN) {
      if (wp == VK_RETURN) wp = VK_SPACE;
      else if (wp == VK_TAB) {
        auto& g = get_group(whole_controls[hw].first); auto i = get_index(g, hw); return focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0), 0;
      } else if (wp == VK_ESCAPE) return SetFocus(main::hwnd), 0; }
    return CallWindowProcW(defproc, hw, msg, wp, lp); }
public:
  const button_state<checkbox, decltype(&get_state)> state;
  void (*enter)(const checkbox& This) = nullptr;
  checkbox() noexcept : control(), state(this, get_state) {}
  checkbox(checkbox&& A) : control(mv(dynamic_cast<control&>(A))), state(this, get_state), enter(A.enter) {}
  checkbox& operator=(checkbox&& A) { return dynamic_cast<control&>(*this) = mv(dynamic_cast<control&>(A)), enter = A.enter, *this; }
  checkbox(const rect& Rect, const stv2& Text, nat4 ButtonStyle = {})
    : control(L"BUTTON", Text, BS_AUTOCHECKBOX | ButtonStyle, Rect, true), state(this, get_state) {
    if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC); SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); }
  checkbox(natt GroupNo, const rect& Rect, const stv2& Text, nat4 ButtonStyle = {})
    : control(L"BUTTON", Text, BS_AUTOCHECKBOX | ButtonStyle, Rect, GroupNo, true), state(this, get_state) {
    if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC); SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); }
};

/// class for creating radiobutton controls
class radiobutton : public control {
  nat4 get_index(HWND hw) { for (nat4 i{}; i < buttons.size(); ++i) if (buttons[i] == hw) return i; return nat4(-1); }
  void initialize(const rect& r, const stv2& t, const auto& ts) {
    if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC); SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
    int4 h = r.height() / (1 + extent<decltype(ts)>), w = r.width();
    auto hw = CreateWindowExW(WS_EX_TOPMOST, L"BUTTON", t, WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP, 0, 0, w, h, hwnd, 0, main::hinstance, 0);
    if (!defproc2) defproc2 = (WNDPROC)GetWindowLongPtrW(hw, GWLP_WNDPROC); SetWindowLongPtrW(hw, GWLP_WNDPROC, (LONG_PTR)proc2);
    SendMessageW(hw, WM_SETFONT, (WPARAM)hfont, true); SetWindowLongPtrW(hw, GWLP_ID, (LONG_PTR)hw), buttons.emplace_back(hw);
    [&]<natt... Is>(sequence<Is...>) {
      (([&]<natt I>(constant<I>) {
        auto hw = CreateWindowExW(WS_EX_TOPMOST, L"BUTTON", get<I>(ts), WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 0, int4(I) * h, w, h, hwnd, 0, main::hinstance, 0);
        SetWindowLongPtrW(hw, GWLP_WNDPROC, (LONG_PTR)proc2), SendMessageW(hw, WM_SETFONT, (WPARAM)hfont, true);
        SetWindowLongPtrW(hw, GWLP_ID, (LONG_PTR)hw), buttons.emplace_back(hw); }(constant<Is>{})), ...); };
    SendMessageW(buttons[0], BM_SETCHECK, BST_CHECKED, 0);
  }
protected:
  array<HWND> buttons{};
  inline static WNDPROC defproc = nullptr, defproc2 = nullptr;
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_COMMAND && HIWORD(wp) == BN_CLICKED) {
      auto& t = *static_cast<radiobutton*>(whole_controls[hw].second); const_cast<nat4&>(t.state) = t.get_index((HWND)lp); if (t.enter) t.enter(t); }
    return CallWindowProcW(defproc, hw, msg, wp, lp); }
  static LRESULT CALLBACK proc2(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_LBUTTONUP) {
      if (auto& t = *static_cast<radiobutton*>(whole_controls[GetParent(hw)].second); t.enter)
        return [&](auto r) { return t.enter(t), r; }(defproc(hw, msg, wp, lp));
    } else if (msg == WM_KEYUP) {
      if (wp == VK_RETURN || wp == VK_SPACE)
        if (auto& t = *static_cast<radiobutton*>(whole_controls[GetParent(hw)].second); t.enter)
          return [&](auto r) { return t.enter(t), r; }(defproc(hw, msg, wp, lp));
    } else if (msg == WM_KEYDOWN) {
      if (wp == VK_RETURN) wp = VK_SPACE;
      else if (wp == VK_TAB) {
        auto p = GetParent(hw); auto& g = get_group(whole_controls[p].first); auto i = control::get_index(g, p);
        return focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0), 0;
      } else if (wp == VK_UP || wp == VK_LEFT) {
        auto& t = *static_cast<radiobutton*>(whole_controls[GetParent(hw)].second);
        SetFocus(t.buttons[(t.get_index(hw) + t.buttons.size() - 1) % t.buttons.size()]);
      } else if (wp == VK_DOWN || wp == VK_RIGHT) {
        auto& t = *static_cast<radiobutton*>(whole_controls[GetParent(hw)].second);
        SetFocus(t.buttons[(t.get_index(hw) + 1) % t.buttons.size()]);
      } else if (wp == VK_ESCAPE) return SetFocus(main::hwnd), 0; }
    return CallWindowProcW(defproc2, hw, msg, wp, lp); }
public:
  const nat4 state{}, count{};
  void (*enter)(const radiobutton& This) = nullptr;
  radiobutton() noexcept = default;
  radiobutton(radiobutton&& A) : control(mv(dynamic_cast<control&>(A))), buttons(mv(A.buttons)), count(A.count), enter(A.enter) {}
  radiobutton& operator=(radiobutton&& A) {
    dynamic_cast<control&>(*this) = mv(dynamic_cast<control&>(A));
    return buttons = mv(A.buttons), const_cast<nat4&>(count) = A.count, enter = A.enter, *this; }
  radiobutton(const rect& Rect, const str2& Title, const str2& FirstButtonText, convertible_to<str2> auto&&... ButtonTexts)
    : control(L"BUTTON", Title.data(), WS_BORDER | BS_GROUPBOX, Rect, true), count(1 + sizeof...(ButtonTexts)) {
    initialize(Rect, FirstButtonText, list<>::asref(ButtonTexts...)); }
  radiobutton(natt GroupNo, const rect& Rect, const str2& Title, const str2& FirstButtonText, convertible_to<str2> auto&&... ButtonTexts)
    : control(L"BUTTON", Title.data(), WS_BORDER | BS_GROUPBOX, Rect, GroupNo, true), count(1 + sizeof...(ButtonTexts)) {
    initialize(Rect, FirstButtonText, list<>::asref(ButtonTexts...)); }
  virtual void setfocus() const override { SetFocus(buttons[state]); }
};

/// font class
template<value Size = value{}, string_view Name = L"Yu Gothic UI", intt Alignment = 0, bool Bold = false, bool Italic = false> class font;

template<stv2 Name, intt Alignment, bool Bold, bool Italic> class font<value{}, Name, Alignment, Bold, Italic> {
protected:
  font(const font&) = delete;
  font& operator=(const font&) = delete;
  comptr<IDWriteTextFormat> dw_format{};
public:
  font() noexcept = default;
  explicit operator bool() const noexcept { return bool(dw_format); }
  operator comptr<IDWriteTextFormat>::reference*() const noexcept { ywlib_assert(dw_format, "this font is not valid"); return dw_format; }
  font(fat4 Size, const stv2& Name = L"Yu Gothic UI", intt Alignment = 0, bool Bold = false, bool Italic = false) {
    tiff(main::dw_factory->CreateTextFormat(
      Name.data(), nullptr, Bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
      Italic ? DWRITE_FONT_STYLE_OBLIQUE : DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, fat4(Size), L"", dw_format.addressof()));
    if (Alignment < 0) dw_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    else if (Alignment > 0) dw_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
    else dw_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER); }
  fat4 size() const { ywlib_assert(dw_format, "this font is not valid"); return dw_format->GetFontSize(); }
  str2 name() const {
    ywlib_assert(dw_format, "this font is not valid");
    str2 out(dw_format->GetFontFamilyNameLength(), 0); tiff(dw_format->GetFontFamilyName(out.data(), out.size() + 1)); return out; }
  intt alignment() const {
    ywlib_assert(dw_format, "this font is not valid");
    if (auto a = dw_format->GetTextAlignment(); a == DWRITE_TEXT_ALIGNMENT_LEADING) return -1;
    else return a == DWRITE_TEXT_ALIGNMENT_TRAILING ? 1 : 0; }
  void alignment(intt Alignment) {
    if (Alignment < 0) dw_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    else if (Alignment > 0) dw_format->SetTextAlignment(Alignment > 0 ? DWRITE_TEXT_ALIGNMENT_TRAILING : DWRITE_TEXT_ALIGNMENT_CENTER); }
};

template<value Size, stv2 Name, intt Alignment, bool Bold, bool Italic> requires(Size != value{}) class font<Size, Name, Alignment, Bold, Italic> {
  static comptr<IDWriteTextFormat> _init() {
    comptr<IDWriteTextFormat> f;
    tiff(main::dw_factory->CreateTextFormat(Name.data(), nullptr, Bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
      Italic ? DWRITE_FONT_STYLE_OBLIQUE : DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fat4(Size), L"", f.addressof()));
    if (Alignment < 0) return f->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING), mv(f);
    else return f->SetTextAlignment(Alignment > 0 ? DWRITE_TEXT_ALIGNMENT_TRAILING : DWRITE_TEXT_ALIGNMENT_CENTER), mv(f); }
  static const auto& _get() { static comptr<IDWriteTextFormat> f(_init()); return f; }
public:
  explicit operator bool() const noexcept { return bool(_get()); }
  operator comptr<IDWriteTextFormat>::reference*() const noexcept { return _get(); }
  constexpr fat4 size() const { return Size; }
  constexpr stv2 name() const { return Name; }
  constexpr intt alignment() const { return Alignment; }
};

template<color Color = color::undefined> class brush;

template<> class brush<color::undefined> {
protected:
  brush(const brush&) = delete;
  brush& operator=(const brush&) = delete;
  comptr<ID2D1SolidColorBrush> d2d_brush{};
public:
  brush() noexcept = default;
  explicit operator bool() const noexcept { return bool(d2d_brush); }
  operator comptr<ID2D1SolidColorBrush>::reference*() const noexcept(nodebug) { ywlib_assert(d2d_brush, "this brush is not valid"); return d2d_brush; }
  brush(color Color) { tiff(main::d2d_context->CreateSolidColorBrush(bitcast<D2D1_COLOR_F>(Color), d2d_brush.addressof())); }
  brush& operator=(brush&& Brush) noexcept { return d2d_brush.reset(), d2d_brush.swap(Brush.d2d_brush), *this; }
  yw::color color() const { ywlib_assert(d2d_brush, "this brush is not valid"); return bitcast<yw::color>(d2d_brush->GetColor()); }
  void draw_line(const vector& BeginEnd, fat4 Width) const { main::d2d_context->DrawLine({BeginEnd.x, BeginEnd.y}, {BeginEnd.z, BeginEnd.w}, *this, Width); }
  void draw_rectangle(const vector& Rect) const { main::d2d_context->FillRectangle(reinterpret_cast<const D2D1_RECT_F&>(Rect), *this); }
  void draw_rectangle(const vector& Rect, fat4 Width) const { main::d2d_context->DrawRectangle(reinterpret_cast<const D2D1_RECT_F&>(Rect), *this, Width); }
  void draw_rounded_rectangle(const vector& Rect, fat4 Radius) const {
    main::d2d_context->FillRoundedRectangle(D2D1_ROUNDED_RECT{reinterpret_cast<const D2D1_RECT_F&>(Rect), Radius, Radius}, *this); }
  void draw_rounded_rectangle(const vector& Rect, const list<fat4, fat4>& Radius) const {
    main::d2d_context->FillRoundedRectangle(D2D1_ROUNDED_RECT{reinterpret_cast<const D2D1_RECT_F&>(Rect), Radius.first, Radius.second}, *this); }
  void draw_rounded_rectangle(const vector& Rect, fat4 Width, fat4 Radius) const {
    main::d2d_context->DrawRoundedRectangle(D2D1_ROUNDED_RECT{reinterpret_cast<const D2D1_RECT_F&>(Rect), Radius, Radius}, *this, Width); }
  void draw_rounded_rectangle(const vector& Rect, fat4 Width, const list<fat4, fat4>& Radius) const {
    main::d2d_context->DrawRoundedRectangle(D2D1_ROUNDED_RECT{reinterpret_cast<const D2D1_RECT_F&>(Rect), Radius.first, Radius.second}, *this, Width); }
  void draw_text(const vector& Rect, convertible_to<IDWriteTextFormat*> auto&& Font, const stv2 Text) const {
    main::d2d_context->DrawTextW(Text.data(), nat4(Text.size()), Font, reinterpret_cast<const D2D1_RECT_F&>(Rect), *this); }
};

template<color Color> requires(Color != color::undefined) class brush<Color> {
  static const auto& _get() { static auto c = yw::brush<color::undefined>(Color); return c; }
public:
  explicit operator bool() const noexcept { return bool(_get()); }
  operator comptr<ID2D1SolidColorBrush>::reference*() const noexcept { return _get(); }
  yw::color color() const { return Color; }
  void draw_line(const vector& BeginEnd, fat4 Width) const { _get().draw_line(BeginEnd, Width); }
  void draw_rectangle(const vector& Rect) const { _get().draw_rectangle(Rect); }
  void draw_rectangle(const vector& Rect, fat4 Width) const { _get().draw_rectangle(Rect, Width); }
  void draw_rounded_rectangle(const vector& Rect, fat4 Radius) const { _get().draw_rounded_rectangle(Rect, Radius); }
  void draw_rounded_rectangle(const vector& Rect, const list<fat4, fat4>& Radius) const { _get().draw_rounded_rectangle(Rect, Radius); }
  void draw_rounded_rectangle(const vector& Rect, fat4 Width, fat4 Radius) const { _get().draw_rounded_rectangle(Rect, Width, Radius); }
  void draw_rounded_rectangle(const vector& Rect, fat4 Width, const list<fat4, fat4>& Radius) const { _get().draw_rounded_rectangle(Rect, Width, Radius); }
  void draw_text(const vector& Rect, convertible_to<IDWriteTextFormat*> auto&& Font, const stv2 Text) const { _get().draw_text(Rect, Font, Text); }
};

template<typename... Ts> brush(Ts&&...) -> brush<color::undefined>;

template<typename T> class staging_buffer;

template<typename T> class buffer {
protected:
  comptr<ID3D11Buffer> d3d_buffer;
  buffer(const buffer&) = delete;
  buffer& operator=(const buffer&) = delete;
  buffer(natt Count) noexcept : count(static_cast<nat4>(Count)) {}
public:
  using value_type = T;
  const nat4 count{};
  buffer() noexcept = default;
  buffer(buffer&& B) noexcept : d3d_buffer{mv(B.d3d_buffer)}, count{B.count} {}
  buffer& operator=(buffer&& B) noexcept { d3d_buffer = mv(B.d3d_buffer); const_cast<nat4&>(count) = B.count; return *this; }
  explicit operator bool() const noexcept { return bool(d3d_buffer); }
  operator comptr<ID3D11Buffer>::reference*() const noexcept(nodebug) { ywlib_assert(d3d_buffer, "this buffer is not valid"); return d3d_buffer; }
  void from(const buffer& Src) {
    ywlib_assert(d3d_buffer, "this buffer is not valid");
    ywlib_assert(count == Src.count, "the size of the source buffer must be the same as this buffer");
    main::d3d_context->CopyResource(*this, Src); }
  array<T> to_cpu() const;
  array<T> to_cpu(staging_buffer<T>& Staging) const;
};

template<typename T> class staging_buffer : public buffer<T> {
public:
  using buffer<T>::from;
  staging_buffer() noexcept = default;
  explicit staging_buffer(const buffer<T>& Src) : staging_buffer(Src.count) { buffer<T>::from(Src); }
  explicit staging_buffer(natt Count) : buffer<T>(Count) {
    D3D11_BUFFER_DESC desc{nat4(sizeof(T) * Count), D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ};
    tiff(main::d3d_device->CreateBuffer(&desc, nullptr, buffer<T>::d3d_buffer.addressof())); }
  array<T> to_cpu() const {
    ywlib_assert(*this, "this buffer is not valid");
    array<T> a(buffer<T>::count); D3D11_MAPPED_SUBRESOURCE mapped;
    tiff(main::d3d_context->Map(*this, 0, D3D11_MAP_READ, 0, &mapped));
    memcpy(a.data(), mapped.pData, nat4(sizeof(T)) * buffer<T>::count);
    main::d3d_context->Unmap(*this, 0); return a; }
};

template<typename T> array<T> buffer<T>::to_cpu() const { ywlib_assert(*this, "this buffer is not valid"); return staging_buffer<T>(*this).to_cpu(); }
template<typename T> array<T> buffer<T>::to_cpu(staging_buffer<T>& S) const { ywlib_assert(*this, "this buffer is not valid"); return S.from(*this), S.to_cpu(); }

/// class for creating constant buffers
template<typename T> requires((sizeof(T) & 0xf) == 0) class constant_buffer : public buffer<T> {
public:
  using buffer<T>::from;
  constant_buffer() noexcept = default;
  constant_buffer(const T& Value) : buffer<T>(1) {
    static constexpr D3D11_BUFFER_DESC desc{nat4(sizeof(T)), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE};
    [&](D3D11_SUBRESOURCE_DATA d) { tiff(main::d3d_device->CreateBuffer(&desc, &d, buffer<T>::d3d_buffer.addressof())); }({&Value}); }
  template<typename U> requires assignable<T&, U> || vassignable<T&, U> void from(U&& Value) {
    if (*this) { D3D11_MAPPED_SUBRESOURCE mapped; tiff(main::d3d_context->Map(*this, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
                 if constexpr (assignable<T&, U>) *reinterpret_cast<T*>(mapped.pData) = fwd<U>(Value);
                 else vassign(*reinterpret_cast<T*>(mapped.pData), fwd<U>(Value));
                 main::d3d_context->Unmap(*this, 0);
    } else {     T temp;
                 if constexpr (assignable<T&, U>) *this = constant_buffer(temp = fwd<U>(Value));
                 else vassign(temp, fwd<U>(Value)), *this = constant_buffer(temp); } }
  void from(invocable<T&> auto&& Func) {
    ywlib_assert(*this, "this buffer is not valid");
    if (*this) { D3D11_MAPPED_SUBRESOURCE mapped; tiff(main::d3d_context->Map(*this, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
                 Func(*reinterpret_cast<T*>(mapped.pData)), main::d3d_context->Unmap(*this, 0);
    } else [&](T temp) { Func(temp), *this = constant_buffer(temp); }({}); }
  void to_vs(natt Slot = 0) const { ywlib_assert(*this, "this buffer is not valid"); main::d3d_context->VSSetConstantBuffers(nat4(Slot), 1, &buffer<T>::d3d_buffer); }
  void to_gs(natt Slot = 0) const { ywlib_assert(*this, "this buffer is not valid"); main::d3d_context->GSSetConstantBuffers(nat4(Slot), 1, &buffer<T>::d3d_buffer); }
  void to_ps(natt Slot = 0) const { ywlib_assert(*this, "this buffer is not valid"); main::d3d_context->PSSetConstantBuffers(nat4(Slot), 1, &buffer<T>::d3d_buffer); }
  void to_cs(natt Slot = 0) const { ywlib_assert(*this, "this buffer is not valid"); main::d3d_context->CSSetConstantBuffers(nat4(Slot), 1, &buffer<T>::d3d_buffer); }
};

template<typename T> constant_buffer(const T&) -> constant_buffer<T>;

/// class for creating structured buffers
template<typename T> class structured_buffer : public buffer<T> {
protected:
  comptr<ID3D11ShaderResourceView> d3d_srv;
public:
  using buffer<T>::from;
  structured_buffer() noexcept = default;
  operator comptr<ID3D11ShaderResourceView>::reference*() const noexcept(nodebug) { ywlib_assert(*this, "this buffer is not valid"); return d3d_srv; }
  explicit structured_buffer(const buffer<T>& Src) : structured_buffer(Src.count) { buffer<T>::from(Src); }
  explicit structured_buffer(natt Count) : buffer<T>(Count) {
    D3D11_BUFFER_DESC desc{nat4(sizeof(T) * Count), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, sizeof(T)};
    tiff(main::d3d_device->CreateBuffer(&desc, nullptr, buffer<T>::d3d_buffer.addressof()));
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{DXGI_FORMAT_UNKNOWN, D3D11_SRV_DIMENSION_BUFFER, {}}; srv_desc.Buffer.NumElements = nat4(Count);
    tiff(main::d3d_device->CreateShaderResourceView(*this, &srv_desc, d3d_srv.addressof())); }
  structured_buffer(const T* Data, natt Count) : buffer<T>(Count) {
    D3D11_BUFFER_DESC desc{nat4(sizeof(T) * Count), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, nat4(sizeof(T))};
    if (Data) { D3D11_SUBRESOURCE_DATA data{.pSysMem = Data, .SysMemPitch = nat4(sizeof(T))};
                tiff(main::d3d_device->CreateBuffer(&desc, &data, buffer<T>::d3d_buffer.addressof()));
    } else      tiff(main::d3d_device->CreateBuffer(&desc, nullptr, buffer<T>::d3d_buffer.addressof()));
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{DXGI_FORMAT_UNKNOWN, D3D11_SRV_DIMENSION_BUFFER, D3D11_BUFFER_SRV{0, nat4(Count)}};
    tiff(main::d3d_device->CreateShaderResourceView(*this, &srv_desc, d3d_srv.addressof())); }
  void from(const T* Data) { ywlib_assert(*this, "this buffer is not valid");
                             main::d3d_context->UpdateSubresource(*this, 0, 0, Data, nat4(sizeof(T)), nat4(sizeof(T)) * buffer<T>::count); }
  void to_vs(natt Slot = 0) const { ywlib_assert(*this, "this buffer is not valid"); main::d3d_context->VSSetShaderResources(nat4(Slot), 1, &d3d_srv); }
  void to_gs(natt Slot = 0) const { ywlib_assert(*this, "this buffer is not valid"); main::d3d_context->GSSetShaderResources(nat4(Slot), 1, &d3d_srv); }
  void to_ps(natt Slot = 0) const { ywlib_assert(*this, "this buffer is not valid"); main::d3d_context->PSSetShaderResources(nat4(Slot), 1, &d3d_srv); }
  void to_cs(natt Slot = 0) const { ywlib_assert(*this, "this buffer is not valid"); main::d3d_context->CSSetShaderResources(nat4(Slot), 1, &d3d_srv); }
};

template<typename T> structured_buffer(const buffer<T>&) -> structured_buffer<T>;
template<typename T> structured_buffer(const T*, natt) -> structured_buffer<T>;
template<cnt_range R> structured_buffer(R&&) -> structured_buffer<iter_value<R>>;

/// class for creating unordered access buffers
template<typename T> class unordered_buffer : public buffer<T> {
protected:
  comptr<ID3D11UnorderedAccessView> d3d_uav;
public:
  using buffer<T>::from;
  unordered_buffer() noexcept = default;
  operator comptr<ID3D11UnorderedAccessView>::reference*() const noexcept(nodebug) { ywlib_assert(*this, "this buffer is not valid"); return d3d_uav; }
  explicit unordered_buffer(const buffer<T>& Src) : unordered_buffer(Src.count) { buffer<T>::from(Src); }
  explicit unordered_buffer(natt Count) : buffer<T>(Count) {
    D3D11_BUFFER_DESC desc{nat4(sizeof(T) * Count), D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS, 0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, nat4(sizeof(T))};
    tiff(main::d3d_device->CreateBuffer(&desc, nullptr, buffer<T>::d3d_buffer.addressof()));
    tiff(main::d3d_device->CreateUnorderedAccessView(*this, nullptr, d3d_uav.addressof())); }
  unordered_buffer(const T* Data, natt Count) : buffer<T>(Count) {
    D3D11_BUFFER_DESC desc{nat4(sizeof(T) * Count), D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS, 0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, nat4(sizeof(T))};
    if (Data) { D3D11_SUBRESOURCE_DATA data{.pSysMem = Data, .SysMemPitch = nat4(sizeof(T))};
                tiff(main::d3d_device->CreateBuffer(&desc, &data, buffer<T>::d3d_buffer.addressof()));
    } else      tiff(main::d3d_device->CreateBuffer(&desc, nullptr, buffer<T>::d3d_buffer.addressof()));
    tiff(main::d3d_device->CreateUnorderedAccessView(*this, nullptr, d3d_uav.addressof())); }
  void from(const T* Data) { ywlib_assert(*this, "this buffer is not valid");
                             main::d3d_context->UpdateSubresource(*this, 0, 0, Data, nat4(sizeof(T)), nat4(sizeof(T)) * buffer<T>::count); }
  void to_cs(nat4 Slot = 0) const { ywlib_assert(*this, "this buffer is not valid"); main::d3d_context->CSSetUnorderedAccessViews(Slot, 1, &d3d_uav, nullptr); }
};

template<typename T> unordered_buffer(const buffer<T>&) -> unordered_buffer<T>;
template<typename T> unordered_buffer(const T*, natt) -> unordered_buffer<T>;

/// class for rendering 3D objects
template<specialization_of<typepack> VSResources, specialization_of<typepack> VSCBuffers,
         specialization_of<typepack> GSResources = typepack<>, specialization_of<typepack> GSCBuffers = typepack<>,
         specialization_of<typepack> PSResources = typepack<>, specialization_of<typepack> PSCBuffers = typepack<>> class renderer {
protected:
  static_assert([]<natt... Is>(sequence<Is...>) { return (convertible_to<typename VSResources::template at<Is>, ID3D11ShaderResourceView*> && ...); }(make_indices_for<VSResources>{}));
  static_assert([]<natt... Is>(sequence<Is...>) { return (convertible_to<typename GSResources::template at<Is>, ID3D11ShaderResourceView*> && ...); }(make_indices_for<GSResources>{}));
  static_assert([]<natt... Is>(sequence<Is...>) { return (convertible_to<typename PSResources::template at<Is>, ID3D11ShaderResourceView*> && ...); }(make_indices_for<PSResources>{}));
  static_assert([]<natt... Is>(sequence<Is...>) { return (convertible_to<typename VSCBuffers::template at<Is>, ID3D11Buffer*> && ...); }(make_indices_for<VSCBuffers>{}));
  static_assert([]<natt... Is>(sequence<Is...>) { return (convertible_to<typename GSCBuffers::template at<Is>, ID3D11Buffer*> && ...); }(make_indices_for<GSCBuffers>{}));
  static_assert([]<natt... Is>(sequence<Is...>) { return (convertible_to<typename PSCBuffers::template at<Is>, ID3D11Buffer*> && ...); }(make_indices_for<PSCBuffers>{}));
  comptr<ID3D11VertexShader> d3d_vs{}; comptr<ID3D11GeometryShader> d3d_gs{}; comptr<ID3D11PixelShader> d3d_ps{};
public:
  using vs_resource_list = type_switch<VSResources::count == 0, none, array<ID3D11ShaderResourceView*, VSResources::count>>;
  using gs_resource_list = type_switch<GSResources::count == 0, none, array<ID3D11ShaderResourceView*, GSResources::count>>;
  using ps_resource_list = type_switch<PSResources::count == 0, none, array<ID3D11ShaderResourceView*, PSResources::count>>;
  using vs_cbuffer_list = type_switch<VSCBuffers::count == 0, none, array<ID3D11Buffer*, VSCBuffers::count>>;
  using gs_cbuffer_list = type_switch<GSCBuffers::count == 0, none, array<ID3D11Buffer*, GSCBuffers::count>>;
  using ps_cbuffer_list = type_switch<PSCBuffers::count == 0, none, array<ID3D11Buffer*, PSCBuffers::count>>;
  renderer() noexcept = default;
  explicit operator bool() const noexcept { return bool(d3d_vs) && bool(d3d_ps); }
  renderer(stv1 HLSL) {
    comptr<ID3DBlob> b, e;
    if (0 > D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, "vsmain", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
                       0, b.addressof(), e.addressof())) throw except((cat1*)e->GetBufferPointer());
    tiff(main::d3d_device->CreateVertexShader(b->GetBufferPointer(), b->GetBufferSize(), 0, d3d_vs.addressof()));
    if (b.reset(), e.reset(); 0 > D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, "psmain", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
                                             0, b.addressof(), e.addressof())) throw except((cat1*)e->GetBufferPointer());
    tiff(main::d3d_device->CreatePixelShader(b->GetBufferPointer(), b->GetBufferSize(), 0, d3d_ps.addressof()));
    if (HLSL.find("gsmain") == npos) return;
    if (b.reset(), e.reset(); 0 > D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, "gsmain", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
                                             0, b.addressof(), e.addressof())) throw except((cat1*)e->GetBufferPointer());
    tiff(main::d3d_device->CreateGeometryShader(b->GetBufferPointer(), b->GetBufferSize(), 0, d3d_gs.addressof())); }
  void operator()(natt VertexCounts, vs_resource_list VSResources, vs_cbuffer_list VSCBuffers,
                  gs_resource_list GSResources = {}, gs_cbuffer_list GSCBuffers = {}, ps_resource_list PSResources = {}, ps_cbuffer_list PSCbuffers = {}) const {
    if constexpr (!same_as<vs_resource_list, none>) main::d3d_context->VSSetShaderResources(0, nat4(VSResources.count), VSResources.data());
    if constexpr (!same_as<gs_resource_list, none>) main::d3d_context->GSSetShaderResources(0, nat4(VSResources.count), VSResources.data());
    if constexpr (!same_as<ps_resource_list, none>) main::d3d_context->PSSetShaderResources(0, nat4(VSResources.count), VSResources.data());
    if constexpr (!same_as<vs_cbuffer_list, none>) main::d3d_context->VSSetConstantBuffers(0, nat4(VSCBuffers.count), VSCBuffers.data());
    if constexpr (!same_as<gs_cbuffer_list, none>) main::d3d_context->GSSetConstantBuffers(0, nat4(GSCBuffers.count), GSCBuffers.data());
    if constexpr (!same_as<ps_cbuffer_list, none>) main::d3d_context->PSSetConstantBuffers(0, nat4(PSCbuffers.count), PSCbuffers.data());
    main::d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); main::d3d_context->IASetInputLayout(nullptr);
    main::d3d_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr); main::d3d_context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
    main::d3d_context->VSSetShader(d3d_vs, 0, 0); main::d3d_context->GSSetShader(d3d_gs, 0, 0); main::d3d_context->PSSetShader(d3d_ps, 0, 0);
    main::d3d_context->Draw(nat4(VertexCounts), 0); }
};

template<specialization_of<typepack> Unordered, specialization_of<typepack> Structured = typepack<>, specialization_of<typepack> Constant = typepack<>> class gpgpu {
protected:
  static_assert(Unordered::count != 0);
  comptr<ID3D11ComputeShader> cs{};
public:
  using ub_list = list<>::from_typepack<Unordered, const unordered_buffer<int4>&>;
  using sb_list = list<>::from_typepack<Structured, const structured_buffer<int4>&>;
  using cb_list = list<>::from_typepack<Constant, const constant_buffer<vector>&>;
  gpgpu() noexcept = default;
  gpgpu(stv1 Hlsl, str1 Entry = "csmain", str1 Target = "cs_5_0") {
    try { comptr<ID3DBlob> b, r;
          if (0 > D3DCompile(Hlsl.data(), Hlsl.size(), 0, 0, 0, Entry.data(), Target.data(),
                         D3D10_SHADER_ENABLE_STRICTNESS, 0, b.addressof(), r.addressof())) throw except((cat1*)r->GetBufferPointer());
      std::cout << source{} << std::endl;
      std::cout << main::d3d_device.get() << std::endl;
      std::cout << b->GetBufferSize() << std::endl;
      std::cout << cs.addressof() << std::endl;
      std::cout << source{} << std::endl;
      tiff(main::d3d_device->GetDeviceRemovedReason());
      tiff(main::d3d_device->CreateComputeShader(b->GetBufferPointer(), b->GetBufferSize(), nullptr, cs.addressof()));
      std::cout << source{} << std::endl;
    } catch (const except& E) { std::cout << E.what() << std::endl, throw except(E); }
  }
  void operator()(ub_list UBuffers, sb_list SBuffers, cb_list CBuffers,
                  nat4 ThreadGroupX, nat4 ThreadGroupY = 1, nat4 ThreadGroupZ = 1) const {
    using ubseq = make_indices_for<ub_list>;
    tiff([&]<natt... Is>(sequence<Is...>) { return (bool(get<Is>(UBuffers)) && ...); }(ubseq{}));
    [&]<natt... Is>(sequence<Is...>) { (get<Is>(UBuffers).to_cs(Is), ...); }(ubseq{});
    using sbseq = make_indices_for<sb_list>;
    if constexpr (extent<sb_list> != 0) {
      tiff([&]<natt... Is>(sequence<Is...>) { return (bool(get<Is>(SBuffers)) && ...); }(sbseq{}));
      [&]<natt... Is>(sequence<Is...>) { (get<Is>(SBuffers).to_cs(Is), ...); }(sbseq{});
    }
    using cbseq = make_indices_for<cb_list>;
    if constexpr (extent<cb_list> != 0) {
      tiff([&]<natt... Is>(sequence<Is...>) { return (bool(get<Is>(CBuffers)) && ...); }(cbseq{}));
      [&]<natt... Is>(sequence<Is...>) { (get<Is>(CBuffers).to_cs(Is), ...); }(cbseq{});
    }
    main::d3d_context->CSSetShader(cs, nullptr, 0);
    main::d3d_context->Dispatch(ThreadGroupX, ThreadGroupY, ThreadGroupZ);
    [&]<natt... Is>(ID3D11UnorderedAccessView* t, sequence<Is...>) { (main::d3d_context->CSSetUnorderedAccessViews(Is, 1, &t, 0), ...); }({}, ubseq{});
    [&]<natt... Is>(ID3D11ShaderResourceView* t, sequence<Is...>) { (main::d3d_context->CSSetShaderResources(Is, 1, &t), ...); }({}, sbseq{});
    [&]<natt... Is>(ID3D11Buffer* t, sequence<Is...>) { (main::d3d_context->CSSetConstantBuffers(Is, 1, &t), ...); }({}, cbseq{});
  }
  /// @brief 一次元に並列化したGPGPUを実行する。
  /// @param Parallels 並列数
  /// @note  動作条件：Shaderにおいて`numthreads(1024, 1, 1)`が指定されていること。
  void operator()(natt Parallels, ub_list UBuffers, sb_list SBuffers, cb_list CBuffers) const {
    this->operator()(mv(UBuffers), mv(SBuffers), mv(CBuffers), nat4(Parallels - 1) / 1024u + 1, 1, 1);
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// class for creating 2D bitmaps
class bitmap {
  bitmap(bitmap&) = delete;
  bitmap& operator=(bitmap&) = delete;
  friend void main::resize(natt, natt);
protected:
  comptr<ID2D1Bitmap1> d2d_bitmap;
public:
  /// width
  const nat4 width{};

  /// height
  const nat4 height{};

  /// default constructor
  bitmap() noexcept = default;

  /// move constructor
  bitmap(bitmap&& Bitmap) noexcept : d2d_bitmap{mv(Bitmap.d2d_bitmap)}, width{Bitmap.width}, height{Bitmap.height} {}

  /// creates a bitmap with `Width` and `Height`
  bitmap(natt Width, natt Height) : width(nat4(Width)), height(nat4(Height)) {
    auto p = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET, D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
    tiff(main::d2d_context->CreateBitmap(D2D1::SizeU(width, height), nullptr, 0, p, d2d_bitmap.addressof()));
  }

  /// creates a bitmap from an image file
  explicit bitmap(const path& Image) {
    comptr<IWICBitmapDecoder> decoder{};
    tiff(main::wic_factory->CreateDecoderFromFilename(
      Image.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.addressof()));
    comptr<IWICBitmapFrameDecode> frame;
    tiff(decoder->GetFrame(0, frame.addressof()));
    tiff(frame->GetSize(const_cast<nat4*>(&width), const_cast<nat4*>(&height)));
    WICPixelFormatGUID pixel_format, guid = GUID_WICPixelFormat32bppPRGBA;
    comptr<IWICFormatConverter> fc;
    tiff(frame->GetPixelFormat(&pixel_format));
    tiff(main::wic_factory->CreateFormatConverter(fc.addressof()));
    [&](BOOL b) { tiff(fc->CanConvert(pixel_format, guid, &b)), tiff(b); }({});
    tiff(fc->Initialize(frame, guid, WICBitmapDitherTypeErrorDiffusion, nullptr, 0, WICBitmapPaletteTypeMedianCut));
    tiff(main::d2d_context->CreateBitmapFromWicBitmap(fc, d2d_bitmap.addressof()));
  }

  /// move assignment
  bitmap& operator=(bitmap&& Bitmap) noexcept {
    d2d_bitmap = mv(Bitmap.d2d_bitmap);
    const_cast<nat4&>(width) = Bitmap.width;
    const_cast<nat4&>(height) = Bitmap.height;
    return *this;
  }

  /// checks if this is valid
  explicit operator bool() const noexcept { return bool(d2d_bitmap); }

  /// conversion to a restricted pointer to `ID2D1Bitmap1`
  operator comptr<ID2D1Bitmap1>::reference*() const noexcept(nodebug) {
    ywlib_assert(d2d_bitmap, "this bitmap is not valid"); return d2d_bitmap; }

  /// saves this bitmap as a PNG file
  void to_png(const path& Path) const {
    if (Path.exists()) Path.remove();
    comptr<IWICStream> stream{};
    tiff(main::wic_factory->CreateStream(stream.addressof()));
    tiff(stream->InitializeFromFilename(Path.c_str(), GENERIC_WRITE));
    comptr<IWICBitmapEncoder> encoder{};
    tiff(main::wic_factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, encoder.addressof()));
    tiff(encoder->Initialize(stream, WICBitmapEncoderNoCache));
    comptr<IWICBitmapFrameEncode> frame{};
    tiff(encoder->CreateNewFrame(frame.addressof(), nullptr));
    tiff(frame->Initialize(nullptr));
    comptr<IWICImageEncoder> image_encoder{};
    tiff(main::wic_factory->CreateImageEncoder(main::d2d_device, image_encoder.addressof()));
    tiff(image_encoder->WriteFrame(d2d_bitmap, frame, nullptr));
    frame->Commit(), encoder->Commit(), stream->Commit(STGC_DEFAULT);
  }

  /// begins drawing to this bitmap
  void begin_draw() const { main::d2d_context->SetTarget(d2d_bitmap), main::d2d_context->BeginDraw(); }

  /// begins drawing to this bitmap after filling it with `Color`
  void begin_draw(const color& Color) const { begin_draw(), main::d2d_context->Clear(bitcast<D2D1_COLOR_F>(Color)); }

  /// ends drawing to this bitmap
  void end_draw() const { main::d2d_context->EndDraw(); }

  /// draws this bitmap to another bitmap
  void draw(const vector& Rect, const fat4 Opacity = 1.0f) const {
    main::d2d_context->DrawBitmap(d2d_bitmap, reinterpret_cast<const D2D1_RECT_F&>(Rect), Opacity);
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// class for creating textures
class texture : public bitmap {
  void initialize() {
    comptr<IDXGISurface> surface;
    tiff(operator comptr<ID2D1Bitmap1>::reference*()->GetSurface(surface.addressof()));
    tiff(surface->QueryInterface(d3d_texture.addressof()));
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{
      .Format = DXGI_FORMAT_R8G8B8A8_UNORM, .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D, .Texture2D = {0, 1}};
    tiff(main::d3d_device->CreateShaderResourceView(*this, &srv_desc, d3d_srv.addressof()));
  }
protected:
  comptr<ID3D11Texture2D> d3d_texture;
  comptr<ID3D11ShaderResourceView> d3d_srv;
public:
  /// default constructor
  texture() noexcept = default;

  /// creates a texture with `Width` and `Height`
  texture(natt Width, natt Height) : bitmap(Width, Height) { initialize(); }

  /// creates a texture from an image file
  explicit texture(const path& Image) : bitmap(Image) { initialize(); }

  /// creates a texture from a bitmap
  explicit texture(bitmap&& Bitmap) : bitmap(mv(Bitmap)) { initialize(); }

  /// checks if this is valid
  explicit operator bool() const noexcept { return bool(d3d_texture); }

  /// conversion to a restricted pointer to `ID3D11Texture2D`
  operator comptr<ID3D11Texture2D>::reference*() const noexcept(nodebug) {
    ywlib_assert(d3d_texture, "this texture is not valid");
    return d3d_texture;
  }

  /// conversion to a restricted pointer to `ID3D11ShaderResourceView`
  operator comptr<ID3D11ShaderResourceView>::reference*() const noexcept(nodebug) {
    ywlib_assert(d3d_srv, "this texture is not valid"); return d3d_srv; }

  /// sets this texture to the vertex shader
  void to_vs(natt Slot = 0) const {
    ywlib_assert(*this, "this texture is not valid");
    main::d3d_context->VSSetShaderResources(nat4(Slot), 1, d3d_srv.addressof()); }

  /// sets this texture to the geometry shader
  void to_gs(natt Slot = 0) const {
    ywlib_assert(*this, "this texture is not valid");
    main::d3d_context->GSSetShaderResources(nat4(Slot), 1, d3d_srv.addressof()); }

  /// sets this texture to the pixel shader
  void to_ps(natt Slot = 0) const {
    ywlib_assert(*this, "this texture is not valid");
    main::d3d_context->PSSetShaderResources(nat4(Slot), 1, d3d_srv.addressof()); }

  /// sets this texture to the compute shader
  void to_cs(natt Slot = 0) const {
    ywlib_assert(*this, "this texture is not valid");
    main::d3d_context->CSSetShaderResources(nat4(Slot), 1, d3d_srv.addressof()); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// class for creating 3D render targets
class canvas : public texture {
private:
  void initialize() {
    comptr<ID3D11Texture2D> temp;
    if (const_cast<nat4&>(msaa) = msaa < 2 ? 0 : (msaa < 4 ? 2 : (msaa < 8 ? 4 : 8)); msaa) {
      D3D11_TEXTURE2D_DESC desc{
        width, height, 1, 1, DXGI_FORMAT_D24_UNORM_S8_UINT, {msaa, 0}, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL};
      tiff(main::d3d_device->CreateTexture2D(&desc, nullptr, temp.addressof()));
      tiff(main::d3d_device->CreateDepthStencilView(temp, nullptr, d3d_dsv.addressof()));
      temp.reset(), desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM, desc.BindFlags = D3D11_BIND_RENDER_TARGET;
      tiff(main::d3d_device->CreateTexture2D(&desc, nullptr, temp.addressof()));
      D3D11_RENDER_TARGET_VIEW_DESC rtv_desc{DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_RTV_DIMENSION_TEXTURE2DMS};
      tiff(main::d3d_device->CreateRenderTargetView(temp, &rtv_desc, d3d_rtv.addressof()));
    } else {
      D3D11_TEXTURE2D_DESC desc{
        width, height, 1, 1, DXGI_FORMAT_D24_UNORM_S8_UINT, {1, 0}, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL};
      tiff(main::d3d_device->CreateTexture2D(&desc, nullptr, temp.addressof()));
      tiff(main::d3d_device->CreateDepthStencilView(temp, nullptr, d3d_dsv.addressof()));
      D3D11_RENDER_TARGET_VIEW_DESC rtv_desc{DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_RTV_DIMENSION_TEXTURE2D};
      tiff(main::d3d_device->CreateRenderTargetView(*this, &rtv_desc, d3d_rtv.addressof()));
    }
  }
protected:
  comptr<ID3D11RenderTargetView> d3d_rtv;
  comptr<ID3D11DepthStencilView> d3d_dsv;
public:
  /// sampling count for MSAA
  const nat4 msaa{};

  /// default constructor
  canvas() noexcept = default;

  /// move constructor
  canvas(canvas&& Canvas) noexcept : texture(mv(Canvas)), msaa(Canvas.msaa),
                                     d3d_rtv{mv(Canvas.d3d_rtv)}, d3d_dsv{mv(Canvas.d3d_dsv)} {}

  /// creates a canvas with `Width` and `Height`
  canvas(natt Width, natt Height, natt MSAA = 0) : texture(Width, Height), msaa(nat4(MSAA)) { initialize(); }

  /// creates a canvas from a bitmap
  explicit canvas(texture&& Texture, natt MSAA = 0) : texture(mv(Texture)), msaa(nat4(MSAA)) { initialize(); }

  /// move assignment
  canvas& operator=(canvas&& Canvas) noexcept {
    texture::operator=(mv(Canvas));
    d3d_rtv = mv(Canvas.d3d_rtv);
    d3d_dsv = mv(Canvas.d3d_dsv);
    const_cast<nat4&>(msaa) = Canvas.msaa;
    return *this;
  }

  /// checks if this is valid
  explicit operator bool() const noexcept { return bool(d3d_rtv) && bool(d3d_dsv); }

  /// conversion to a restricted pointer to `ID3D11RenderTargetView`
  operator comptr<ID3D11RenderTargetView>::reference*() const noexcept(nodebug) {
    ywlib_assert(d3d_rtv, "this canvas is not valid");
    return d3d_rtv;
  }

  /// conversion to a restricted pointer to `ID3D11DepthStencilView`
  operator comptr<ID3D11DepthStencilView>::reference*() const noexcept(nodebug) {
    ywlib_assert(d3d_dsv, "this canvas is not valid");
    return d3d_dsv;
  }

  /// begins 3d rendering to this canvas
  template<typename... RTV_OR_UAV> void begin_render(RTV_OR_UAV&&... Views) const
    requires(bool(convertible_to<RTV_OR_UAV, ID3D11RenderTargetView*> ^ convertible_to<RTV_OR_UAV, ID3D11UnorderedAccessView*>) && ...) {
    ywlib_assert(*this, "this canvas is not valid");
    main::d3d_context->ClearDepthStencilView(d3d_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    [](D3D11_VIEWPORT vp) { main::d3d_context->RSSetViewports(1, &vp); }({0, 0, (fat4)width, (fat4)height, 0, 1});
    static constexpr nat4 m = nat4(counts<convertible_to<RTV_OR_UAV, ID3D11RenderTargetView*>...>), n = nat4(sizeof...(RTV_OR_UAV) - m);
    if constexpr (n == 0) [&]<natt... Is>(array<ID3D11RenderTargetView*, m + 1> RTVs) {
      main::d3d_context->OMSetRenderTargets(m + 1, RTVs.data(), d3d_dsv);
    }({d3d_rtv, Views...});
    else [&](auto&& RTVs, auto&& UAVs) {
      main::d3d_context->OMSetRenderTargetsAndUnorderedAccessViews(m + 1, RTVs.data(), d3d_dsv, m + 1, n, UAVs.data(), nullptr);
    }([&]<natt... Is>(sequence<Is...>) { return array<ID3D11RenderTargetView*, m + 1>{d3d_rtv, parameter_switch<Is>(Views...)...}; }(
        cond_indices<sequence<convertible_to<RTV_OR_UAV, ID3D11RenderTargetView*>...>>{}),
      [&]<natt... Is>(sequence<Is...>) { return array<ID3D11UnorderedAccessView*, n>{parameter_switch<Is>(Views...)...}; }(
        cond_indices<sequence<convertible_to<RTV_OR_UAV, ID3D11UnorderedAccessView*>...>>{}));
  };

  /// begins 3d rendering to this canvas after clearing it with `Color`
  template<typename... RTV_OR_UAV> void begin_render(const color& Color, RTV_OR_UAV&&... Views) const
    requires(bool(convertible_to<RTV_OR_UAV, ID3D11RenderTargetView*> ^ convertible_to<RTV_OR_UAV, ID3D11UnorderedAccessView*>) && ...) {
    ywlib_assert(*this, "this canvas is not valid");
    main::d3d_context->ClearRenderTargetView(d3d_rtv, &Color.r);
    begin_render(fwd<RTV_OR_UAV>(Views)...);
  }

  /// ends 3d rendering to this canvas
  void end_render() const {
    if (msaa) [&](comptr<ID3D11Resource> a) {
      d3d_rtv->GetResource(a.addressof());
      main::d3d_context->ResolveSubresource(*this, 0, a, 0, DXGI_FORMAT_R8G8B8A8_UNORM); }({});
    main::d3d_context->OMSetRenderTargets(0, nullptr, nullptr);
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// class for creating cameras for 3D rendering
class camera : public canvas {
public:
  /// view matrix
  xmatrix view;

  /// view * projection matrix
  xmatrix view_proj;

  /// offsets of camera lens form the center of body
  vector offset{};

  /// radians to rotate the camera around the x, y, z-axis
  vector rotation{};

  /// position of the camera
  vector position{};

  /// magnification factor
  /// @note If orthographic projection is used, this is the magnification factor of the lens.
  /// @note If perspective projection is used, this is the reciprocal tangent of the half of the field of view.
  fat4 factor = 1.0f;

  /// enables or disables orthographic projection
  bool orthographic = false;

  /// default constructor
  camera() noexcept = default;

  /// creates a camera with `Width` and `Height`
  camera(natt Width, natt Height, natt MSAA = 0) : canvas(Width, Height, MSAA) {}

  /// creates a camera from a canvas
  explicit camera(canvas&& Canvas) : canvas(mv(Canvas)) {}

  /// updates the matrices
  void update() {
    constexpr fat4 f = 1048576.0f, n = 0.25f;
    xvview(position, rotation, offset, view);
    if (orthographic) {
      auto a = xv(-2.0f * factor / width, 2.0f * factor / height, 1.0f / (f - n), n / (f - n));
      view_proj[0] = xvmul(view[0], xvpermute<0, 0, 0, 0>(a));
      view_proj[1] = xvmul(view[1], xvpermute<1, 1, 1, 1>(a));
      view_proj[2] = xvsub(xvmul(view[2], xvpermute<2, 2, 2, 2>(a)), xvsetzero<1, 1, 1, 0>(a));
      view_proj[3] = view[3];
    } else {
      auto a = xv((-factor * height) / width, factor, f / (f - n), -f * n / (f - n));
      view_proj[0] = xvmul(view[0], xvpermute<0, 0, 0, 0>(a));
      view_proj[1] = xvmul(view[1], xvpermute<1, 1, 1, 1>(a));
      view_proj[2] = xvadd(xvmul(view[2], xvpermute<2, 2, 2, 2>(a)), xvsetzero<1, 1, 1, 0>(a));
      view_proj[3] = view[2];
    }
  }
};

namespace main {
namespace system {
inline bitmap render_target{};
}
/// width of the main window
inline const nat4& width = render_target.width;

/// height of the main window
inline const nat4& height = render_target.height;

/// resizes the main window
void main::resize(natt Width, natt Height) {
  static int4 width_pad{}, height_pad{};         // padding for the window frame
  static constexpr cat2 class_name[] = L"ywlib"; // class name of the main window
  system::render_target = bitmap{};                               // resets the render target
  const_cast<nat4&>(system::render_target.width) = nat4(Width);   // updates the width
  const_cast<nat4&>(system::render_target.height) = nat4(Height); // updates the height
  if (username.empty()) {
    DWORD size{};
    ::GetUserNameW(nullptr, &size);
    system::username.resize(size - 1);
    ::GetUserNameW(system::username.data(), &size); }
  if (!system::hinstance) {
    system::hinstance = GetModuleHandleW(0);
    WNDCLASSEXW wc{sizeof(WNDCLASSEXW), CS_OWNDC, wndproc, 0, 0, system::hinstance};
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW), wc.lpszClassName = class_name;
    tiff(RegisterClassExW(&wc)); }
  if (!system::hwnd) {
    tiff(system::hwnd = CreateWindowExW(WS_EX_ACCEPTFILES, class_name, class_name, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN,
                                        0, 0, int4(width), int4(height), 0, 0, system::hinstance, 0));
  if (!hfont) hfont = CreateFontW(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
                                    CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_ROMAN, L"Yu Gothic UI");
    tiff(system::hfont = CreateFontW(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
                                     CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_ROMAN, L"Yu Gothic UI"));
    [&](int c) { auto args = CommandLineToArgvW(GetCommandLineW(), &c); for (system::args.resize(c); 0 <= --c;) system::args[c] = args[c]; }(0);
    [&](RECT r = {}) { GetClientRect(system::hwnd, &r), width_pad = width - r.right, height_pad = height - r.bottom; }();
    tiff(SetWindowPos(system::hwnd, 0, 0, 0, width + width_pad, height + height_pad, SWP_NOZORDER));
    comptr<IDXGIFactory2> factory{};
    tiff(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
    tiff(factory->MakeWindowAssociation(system::hwnd, DXGI_MWA_NO_ALT_ENTER));
    D3D_FEATURE_LEVEL featurelevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};
    tiff(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                           featurelevels, (UINT)extent<decltype(featurelevels)>, D3D11_SDK_VERSION,
                           (ID3D11Device**)&system::d3d_device, nullptr, (ID3D11DeviceContext**)&system::d3d_context));
    DXGI_SWAP_CHAIN_DESC1 sc_desc{width, height, DXGI_FORMAT_R8G8B8A8_UNORM, false, {1, 0}, DXGI_USAGE_RENDER_TARGET_OUTPUT, 2};
    sc_desc.Scaling = DXGI_SCALING_STRETCH, sc_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    tiff(factory->CreateSwapChainForHwnd(system::d3d_device, system::hwnd, &sc_desc, nullptr, nullptr, &system::swap_chain));
    tiff(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &system::d2d_factory));
    [&](comptr<IDXGIDevice2> dxgi_device) {
      tiff(system::d3d_device.as(dxgi_device));
      tiff(system::d2d_factory->CreateDevice(dxgi_device, &system::d2d_device)); }({});
    tiff(system::d2d_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &system::d2d_context));
    tiff(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&system::dw_factory));
    tiff(CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&system::wic_factory)));
    [&](comptr<ID3D11BlendState> state, D3D11_BLEND_DESC desc) {
      desc.RenderTarget[0] = {true, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD,
                                D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, 0x0f};
      tiff(system::d3d_device->CreateBlendState(&desc, &state));
      system::d3d_context->OMSetBlendState(state, nullptr, 0xffffffff); }({}, {});
    [&](comptr<ID3D11SamplerState> state, D3D11_SAMPLER_DESC desc) {
      desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP, desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
      desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP, desc.MaxAnisotropy = 1;
      desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS, desc.MaxLOD = D3D11_FLOAT32_MAX;
      tiff(system::d3d_device->CreateSamplerState(&desc, &state));
      system::d3d_context->PSSetSamplers(0, 1, &asconst(state)); }({}, {D3D11_FILTER_MIN_MAG_MIP_LINEAR});
    [&](comptr<ID3D11RasterizerState> state, D3D11_RASTERIZER_DESC desc) {
      desc.DepthClipEnable = true, desc.MultisampleEnable = true, desc.AntialiasedLineEnable = true;
      tiff(system::d3d_device->CreateRasterizerState(&desc, &state));
      system::d3d_context->RSSetState(state); }({}, {D3D11_FILL_SOLID, D3D11_CULL_NONE, true});
  }
  tiff(SetWindowPos(system::hwnd, 0, 0, 0, width + width_pad, height + height_pad, SWP_NOMOVE | SWP_NOZORDER));
  system::bitmap.reset();
  system::d2d_context->SetTarget(0);
  tiff(system::swap_chain->ResizeBuffers(2, width, height, DXGI_FORMAT_UNKNOWN, 0));
  [&](comptr<ID3D11Texture2D> tex, comptr<IDXGISurface> surface) {
    tiff(system::swap_chain->GetBuffer(0, IID_PPV_ARGS(&tex)));
    tiff(tex->QueryInterface(IID_PPV_ARGS(&surface)));
    auto props = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                                         D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));
    tiff(system::d2d_context->CreateBitmapFromDxgiSurface(surface, props, &system::bitmap)); }({}, {});
}
}



} // clang-format on
