/// @file ywlib/control.h

#pragma once

#include "main.h"

#include <CommCtrl.h>
#pragma comment(lib, "Comctl32.lib")


// clang-format off

namespace yw {

class control;

class label;

class textbox;

class valuebox;

class button;

class checkbox;

class radiobutton;

class progressbar;

template<typename Button, invocable<const Button&> Converter> class button_state;
template<typename B, typename F> button_state(const B*, F&&) -> button_state<B, F>;

}

/// base class for creating a control
class yw::control {
public:
  /// maximum number of control groups
  inline static constexpr natt max_groups = 16;

  /// window handle of this control
  const HWND hwnd{};

  /// group index of this control
  const natt group{npos};

  /// shows this control
  virtual void show() const { ywlib_assert(hwnd, "This control is not initialized"); ShowWindow(hwnd, SW_SHOW); }

  /// hides this control
  virtual void hide() const { ywlib_assert(hwnd, "This control is not initialized"); ShowWindow(hwnd, SW_HIDE); }

  /// sets the focus to this control
  virtual void setfocus() const { ywlib_assert(hwnd, "SetFocus failed"); SetFocus(hwnd); }

  /// sets the null-terminated text to this control
  virtual void text(const stv2& Text) const {
    ywlib_assert(hwnd, "This control is not initialized"); SetWindowTextW(hwnd, Text.data()); }

  /// gets the text of this control
  virtual str2 text() const {
    ywlib_assert(hwnd, "This control is not initialized");
    str2 out(GetWindowTextLengthW(hwnd), {}); GetWindowTextW(hwnd, out.data(), int4(out.size() + 1)); return out; }

  /// shows all controls in the specified group
  static void show(natt GroupNo) {
    ywlib_assert(GroupNo < max_groups, "GroupNo must be less than max_groups");
    for (auto& g : groups[GroupNo + 1]) g.second->show(); }

  /// hides all controls in the specified group
  static void hide(natt GroupNo) {
    ywlib_assert(GroupNo < max_groups, "GroupNo must be less than max_groups");
    for (auto& g : groups[GroupNo + 1]) g.second->hide(); }

  /// destructor
  ~control() noexcept {
    if (!hwnd) return; DestroyWindow(hwnd), whole_controls.erase(hwnd);
    groups[group + 1].erase(groups[group + 1].begin() + get_index(groups[group + 1], hwnd));  }

  /// default constructor
  control() noexcept = default;

  /// move constructor
  control(control&& A) noexcept : hwnd(exchange(const_cast<HWND&>(A.hwnd), nullptr)), group(A.group) {
    whole_controls[hwnd].second = this, groups[group + 1][get_index(groups[group + 1], hwnd)].second = this; }

  /// move assignment operator
  control& operator=(control&& A) {
    if (auto& g = groups[group + 1]; hwnd)
      DestroyWindow(hwnd), g.erase(g.begin() + get_index(g, hwnd)), whole_controls.erase(hwnd);
    const_cast<HWND&>(hwnd) = exchange(const_cast<HWND&>(A.hwnd), nullptr), const_cast<natt&>(group) = A.group;
    if (hwnd) whole_controls[hwnd].second = this, groups[group + 1][get_index(groups[group + 1], hwnd)].second = this;
    return *this; }

  /// checks if this control is initialized
  explicit operator bool() const noexcept { return hwnd; }

protected:
  /// control groups; Each group contains handles, pointers to controls and inputability of controls.
  inline static array<array<list<HWND, control*, bool>>, max_groups + 1> groups;

  /// set of whole controls to provide the group index and the pointer to the control from the window handle
  inline static std::map<HWND, list<natt, control*>> whole_controls;

  /// protected constructor for creating a control which is not attached to any group
  control(const cat2* Class, const stv2& Text, nat4 Style, const rect& Rect, bool Inputable)
    : hwnd(CreateWindowExW(0, Class, Text.data(), WS_CHILD | WS_GROUP | Style, Rect.left, Rect.top,
                           Rect.width(), Rect.height(), main::hwnd, 0, main::hinstance, this)), group(npos) {
    SendMessageW(hwnd, WM_SETFONT, (WPARAM)main::system::hfont, true); SetWindowLongPtrW(hwnd, GWLP_ID, (LONG_PTR)hwnd);
    if (groups[0].empty()) groups[0].reserve(1024);
    groups[0].push_back(list{hwnd, this, Inputable}), whole_controls[hwnd] = list{npos, this}; }

  /// protected constructor for creating a control which is attached to the specified group
  control(const cat2* Class, const stv2& Text, nat4 Style, const rect& Rect, const natt GroupNo, bool Inputable)
    : hwnd(CreateWindowExW(0, Class, Text.data(), WS_CHILD | WS_GROUP | Style, Rect.left, Rect.top,
                           Rect.width(), Rect.height(), main::hwnd, 0, main::hinstance, this)), group(GroupNo) {
    ywlib_assert(GroupNo < max_groups, "Control group index is out of range");
    SendMessageW(hwnd, WM_SETFONT, (WPARAM)main::system::hfont, true), SetWindowLongPtrW(hwnd, GWLP_ID, (LONG_PTR)hwnd);
    if (groups[GroupNo + 1].empty()) groups[GroupNo + 1].reserve(1024);
    groups[GroupNo + 1].push_back(list{hwnd, this, Inputable}), whole_controls[hwnd] = list{GroupNo, this}; }

  /// gets the reference to the control group
  static array<list<HWND, control*, bool>>& get_group(const natt GroupNo) {
    ywlib_assert(GroupNo < max_groups, "Control group index is out of range"); return groups[GroupNo + 1]; }

  /// gets the index of the control in the control group
  static natt get_index(const array<list<HWND, control*, bool>>& Group, HWND Handle) {
    for (natt i{}; i < Group.size(); ++i) if (Group[i].first == Handle) return i;
    ywlib_assert(false, "Control not found"); return npos; }

  /// gets the reference to the control from the window handle
  template<typename Control> static Control& get_control(const HWND Handle) {
    return *static_cast<Control*>(whole_controls[Handle].second); }

  /// sets the focus to the next/prev control in the control group
  static void focus_on_next(const array<list<HWND, control*, bool>>& Group, natt Now, bool Prev) {
    if (Prev) { for (natt j = Now - 1; j < Now; --j) if (Group[j].third) return Group[j].second->setfocus();
                for (natt j = Group.size() - 1; Now < j; --j) if (Group[j].third) return Group[j].second->setfocus();
    } else {    for (natt j = Now + 1; j < Group.size(); ++j) if (Group[j].third) return Group[j].second->setfocus();
                for (natt j = 0; j < Now; ++j) if (Group[j].third) return Group[j].second->setfocus(); } }
};

/// class for creating a label control
class yw::label : public yw::control {
public:
  /// default constructor
  label() noexcept = default;

  /// constructor for creating a label which is not attached to any group
  label(const rect& Rect, const stv2& Text, nat4 StaticStyle = {})
    : control(L"STATIC", Text, StaticStyle, Rect, false) {}

  /// constructor for creating a label which is attached to the specified group
  label(const natt GroupNo, const rect& Rect, const stv2& Text, nat4 StaticStyle = {})
    : control(L"STATIC", Text, StaticStyle, Rect, GroupNo, false) {}
};

/// class for creating textbox controls
class yw::textbox : public yw::control {
protected:
  /// default window procedure
  inline static WNDPROC defproc = nullptr;

  /// window procedure for textbox controls
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CHAR:
      switch (wp) {
        case VK_RETURN:
          if (auto& t = get_control<textbox>(hw); t.input) t.input(t);
          return 0;
        case VK_TAB: {
          auto& g = get_group(whole_controls[hw].first);
          auto i = get_index(g, hw);
          auto& t = *(textbox*)(g[i].second);
          return (t.tab ? t.tab(t) : focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0)), 0; }
        case VK_ESCAPE: return SetFocus(main::hwnd), 0;
      } break;
    case WM_SETFOCUS:
      if (auto& t = get_control<textbox>(hw); t.intofocus) t.intofocus(t);
      return 0;
    case WM_KILLFOCUS:
      if (auto& t = get_control<textbox>(hw); t.killfocus) t.killfocus(t);
      return 0;
    } return CallWindowProcW(defproc, hw, msg, wp, lp); }
public:
  /// callback function for the enter key
  void (*input)(const textbox& This) = nullptr;

  /// callback function for the tab key
  void (*tab)(const textbox& This) = nullptr;

  /// callback function for the focus event
  void (*intofocus)(const textbox& This) = nullptr;

  /// callback function for the killfocus event
  void (*killfocus)(const textbox& This) = nullptr;

  /// default constructor
  textbox() noexcept = default;

  /// constructor for creating a textbox which is not attached to any group
  textbox(const rect& Rect, const stv2& Init = L"", nat4 EditStyle = {})
    : control(L"EDIT", Init, EditStyle, Rect, true) {
    if (!defproc) defproc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); }

  /// constructor for creating a textbox which is attached to the specified group
  textbox(const natt GroupNo, const rect& Rect, const stv2& Init, nat4 EditStyle = {})
    : control(L"EDIT", Init, EditStyle, Rect, GroupNo, true) {
    if (!defproc) defproc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); }
};

/// class for creating a valuebox control
class yw::valuebox : public yw::control {
protected:
  /// default window procedure
  inline static WNDPROC defproc = nullptr;

  /// window procedure for valuebox controls
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CHAR:
      switch (wp) {
      case VK_RETURN:
        if (auto& t = get_control<valuebox>(hw); t.input) t.input(t);
        return 0;
      case VK_TAB: {
        auto& g = get_group(whole_controls[hw].first);
        auto i = get_index(g, hw);
        auto& t = *(valuebox*)(g[i].second);
        return (t.tab ? t.tab(t) : focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0)), 0; }
      case VK_ESCAPE: return SetFocus(main::hwnd), 0;
      case VK_BACK: case VK_DELETE: case VK_LEFT: case VK_RIGHT: return CallWindowProcW(defproc, hw, msg, wp, lp);
      } return GetKeyState(VK_CONTROL) || ('0' <= wp && wp <= '9') || wp == '-' || wp == '.' ? CallWindowProcW(defproc, hw, msg, wp, lp) : 0;
    case WM_SETFOCUS:
      if (auto& t = get_control<valuebox>(hw); t.intofocus) t.intofocus(t);
      else SendMessageW(hw, EM_SETSEL, 0, -1);
      break;
    case WM_KILLFOCUS:
      if (auto& t = get_control<valuebox>(hw); t.killfocus) t.killfocus(t);
      else SendMessageW(hw, EM_SETSEL, 0, -1);
      break;
    } return CallWindowProcW(defproc, hw, msg, wp, lp); }
public:
  /// callback function for the enter key
  void (*input)(const valuebox& This) = nullptr;

  /// callback function for the tab key
  void (*tab)(const valuebox& This) = nullptr;

  /// callback function for the focus event
  void (*intofocus)(const valuebox& This) = nullptr;

  /// callback function for the killfocus event
  void (*killfocus)(const valuebox& This) = nullptr;

  /// default constructor
  valuebox() noexcept = default;

  /// constructor for creating a valuebox which is not attached to any group
  valuebox(const rect& Rect, stv2 Init, nat4 EditStyle = {})
    : control(L"EDIT", Init, EditStyle, Rect, true) {
    if (!defproc) defproc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); }

  /// constructor for creating a valuebox which is attached to the specified group
  valuebox(const natt GroupNo, const rect& Rect, stv2 Init, nat4 EditStyle = {})
    : control(L"EDIT", Init, EditStyle, Rect, GroupNo, true) {
    if (!defproc) defproc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); }

  /// sets the value to this valuebox
  void value(const arithmetic auto Value) const { SetWindowTextW(hwnd, vtos<cat2>(Value).data()); }

  /// gets the value of this valuebox
  template<arithmetic T> T value() const { return stov<T>(text()); }
};

/// class for obtaining the button state
template<typename Button, yw::invocable<const Button&> Converter> class yw::button_state {
  const Button* ptr;
  Converter func;
public:
  template<typename F> button_state(const Button* B, F&& Func) noexcept : ptr(B), func(fwd<F>(Func)) {}
  operator invoke_result<Converter, const Button&>() const { return func(*ptr); }
};

/// class for creating button controls
class yw::button : public yw::control {
  static bool get_state(const button& This) { return bool(defproc(This.hwnd, BM_GETSTATE, 0, 0) & BST_PUSHED); }
protected:
  /// default window procedure
  inline static WNDPROC defproc = nullptr;

  /// window procedure for button controls
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_LBUTTONDOWN:
      if (auto& t = get_control<button>(hw); t.input) {
        auto r = CallWindowProcW(defproc, hw, msg, wp, lp);
        t.input(t); return r; } return 0;
    case WM_KEYDOWN:
      switch (wp) {
        case VK_RETURN: case VK_SPACE:
          if (auto& t = get_control<button>(hw); t.input) {
            auto r = CallWindowProcW(defproc, hw, msg, wp, lp);
            t.input(t); return r; } return 0;
        case VK_TAB: {
          auto& g = get_group(whole_controls[hw].first);
          auto i = get_index(g, hw);
          focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0); return 0; }
        case VK_ESCAPE: return SetFocus(main::hwnd), 0; } }
    return CallWindowProcW(defproc, hw, msg, wp, lp); }
public:
  /// checks the state of this button
  const button_state<button, decltype(&get_state)> state;

  /// callback function for the enter key
  void (*input)(const button& This) = nullptr;

  /// default constructor
  button() noexcept : control(), state(this, get_state) {}

  /// move constructor
  button(button&& A) : control(mv(dynamic_cast<control&>(A))), state(this, get_state), input(A.input) {}

  /// move assignment operator
  button& operator=(button&& A) { return (control&)(*this) = mv((control&)(A)), input = A.input, *this; }

  /// constructor for creating a button which is not attached to any group
  button(const rect& Rect, const stv2& Text, nat4 ButtonStyle = {})
    : control(L"BUTTON", Text, ButtonStyle, Rect, true), state(this, get_state) {
    if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); }

  /// constructor for creating a button which is attached to the specified group
  button(natt GroupNo, const rect& Rect, const stv2& Text, nat4 ButtonStyle = {})
    : control(L"BUTTON", Text, ButtonStyle, Rect, GroupNo, true), state(this, get_state) {
    if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); }
};

/// class for creating checkbox controls
class yw::checkbox : public yw::control {
  static bool get_state(const checkbox& This) { return bool(defproc(This.hwnd, BM_GETCHECK, 0, 0) & BST_CHECKED); }
protected:
  /// default window procedure
  inline static WNDPROC defproc = nullptr;

  /// window procedure for checkbox controls
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_LBUTTONUP) {
      if (auto& t = get_control<checkbox>(hw); t.input) { auto r = defproc(hw, msg, wp, lp); t.input(t); return r; }
    } else if (msg == WM_KEYUP) {
      if (wp == VK_RETURN || wp == VK_SPACE)
        if (auto& t = get_control<checkbox>(hw); t.input) { auto r = defproc(hw, msg, VK_SPACE, lp); t.input(t); return r; }
    } else if (msg == WM_KEYDOWN) {
      if (wp == VK_RETURN) wp = VK_SPACE;
      else if (wp == VK_TAB) { auto& g = get_group(whole_controls[hw].first); auto i = get_index(g, hw);
                               return focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0), 0;
      } else if (wp == VK_ESCAPE) return SetFocus(main::hwnd), 0; }
    return CallWindowProcW(defproc, hw, msg, wp, lp); }
public:
  /// checks the state of this checkbox
  const button_state<checkbox, decltype(&get_state)> state;

  /// callback function for the enter key
  void (*input)(const checkbox& This) = nullptr;

  /// default constructor
  checkbox() noexcept : control(), state(this, get_state) {}

  /// move constructor
  checkbox(checkbox&& A) : control(mv(dynamic_cast<control&>(A))), state(this, get_state), input(A.input) {}

  /// move assignment operator
  checkbox& operator=(checkbox&& A) { return (control&)(*this) = mv((control&)(A)), input = A.input, *this; }

  /// constructor for creating a checkbox which is not attached to any group
  checkbox(const rect& Rect, const stv2& Text, nat4 ButtonStyle = {})
    : control(L"BUTTON", Text, BS_AUTOCHECKBOX | ButtonStyle, Rect, true), state(this, get_state) {
    if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); }

  /// constructor for creating a checkbox which is attached to the specified group
  checkbox(natt GroupNo, const rect& Rect, const stv2& Text, nat4 ButtonStyle = {})
    : control(L"BUTTON", Text, BS_AUTOCHECKBOX | ButtonStyle, Rect, GroupNo, true), state(this, get_state) {
    if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc); }
};

/// class for creating radiobutton controls
class yw::radiobutton : public yw::control {
  nat4 get_index(auto hw, nat4 i = 0) { return buttons[i] == (HWND)hw ? i : get_index(hw, i + 1); }
  template<natt N> void initialize(const rect& r, const array<stv2, N>& ts) {
    if (!defproc1) defproc1 = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
    int4 h = r.height() / int4(N), w = r.width();
    auto hw = CreateWindowExW(0, L"BUTTON", ts[0].data(), WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,
                              0, 0, w, h, hwnd, 0, main::hinstance, 0);
    if (!defproc2) defproc2 = (WNDPROC)GetWindowLongPtrW(hw, GWLP_WNDPROC);
    SetWindowLongPtrW(hw, GWLP_WNDPROC, (LONG_PTR)proc2); SetWindowLongPtrW(hw, GWLP_ID, (LONG_PTR)hw);
    SendMessageW(hw, WM_SETFONT, (WPARAM)main::system::hfont, true); buttons.emplace_back(hw);
    for (natt i{1}; i < N; ++i) {
      auto hw = CreateWindowExW(0, L"BUTTON", ts[i].data(), WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
                                0, int4(i) * h, w, h, hwnd, 0, main::hinstance, 0);
      SetWindowLongPtrW(hw, GWLP_WNDPROC, (LONG_PTR)proc2); SetWindowLongPtrW(hw, GWLP_ID, (LONG_PTR)hw);
      SendMessageW(hw, WM_SETFONT, (WPARAM)main::system::hfont, true); buttons.emplace_back(hw); }
    SendMessageW(buttons[0], BM_SETCHECK, BST_CHECKED, 0); }
protected:
  /// array of button handles
  array<HWND> buttons{};

  /// default window procedure for the groupbox
  inline static WNDPROC defproc1 = nullptr;

  /// default window procedure for the radio buttons
  inline static WNDPROC defproc2 = nullptr;

  /// window procedure for the groupbox
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    // if (msg == WM_COMMAND) std::cout << HIWORD(wp) << std::endl;
    if (msg == WM_COMMAND && HIWORD(wp) == BN_CLICKED) {
      auto& t = get_control<radiobutton>(hw); const_cast<nat4&>(t.state) = t.get_index(lp); if (t.input) t.input(t); }
    return CallWindowProcW(defproc1, hw, msg, wp, lp); }

  /// window procedure for the radio buttons
  static LRESULT CALLBACK proc2(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_KEYUP) { if (wp == VK_RETURN || wp == VK_SPACE) return CallWindowProcW(defproc2, hw, msg, wp, lp); }
    else if (msg == WM_KEYDOWN) {
      if (wp == VK_RETURN) wp = VK_SPACE;
      else if (wp == VK_TAB) {
        auto p = GetParent(hw); auto& g = get_group(whole_controls[p].first); auto i = control::get_index(g, p);
        return focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0), 0;
      } else if (wp == VK_UP || wp == VK_LEFT) {
        SendMessageW(hw, BM_SETCHECK, BST_UNCHECKED, 0);
        auto& t = get_control<radiobutton>(GetParent(hw));
        auto b = t.buttons[(t.get_index(hw) + t.buttons.size() - 1) % t.buttons.size()];
        SetFocus(b); SendMessageW(b, BM_SETCHECK, BST_CHECKED, 0);
      } else if (wp == VK_DOWN || wp == VK_RIGHT) {
        SendMessageW(hw, BM_SETCHECK, BST_UNCHECKED, 0);
        auto& t = get_control<radiobutton>(GetParent(hw));
        auto b = t.buttons[(t.get_index(hw) + 1) % t.buttons.size()];
        SetFocus(b); SendMessageW(b, BM_SETCHECK, BST_CHECKED, 0);
      } else if (wp == VK_ESCAPE) return SetFocus(main::hwnd), 0; }
    return CallWindowProcW(defproc2, hw, msg, wp, lp); }
public:
  /// index of the selected button
  const nat4 state{};

  /// number of buttons
  const nat4 count{};

  /// callback function for the enter key
  void (*input)(const radiobutton& This) = nullptr;

  /// default constructor
  radiobutton() noexcept = default;

  /// move constructor
  radiobutton(radiobutton&& A) : control(mv(dynamic_cast<control&>(A))),
                                 buttons(mv(A.buttons)), count(A.count), input(A.input) {}

  /// move assignment operator
  radiobutton& operator=(radiobutton&& A) {
    dynamic_cast<control&>(*this) = mv(dynamic_cast<control&>(A));
    return buttons = mv(A.buttons), const_cast<nat4&>(count) = A.count, input = A.input, *this; }

  /// constructor for creating a radiobutton which is not attached to any group
  template<tuple Tp> radiobutton(const rect& Rect, const stv2& Title, Tp&& Texts, nat4 BoxStyle = {})
    requires buildable<array<stv2, extent<Tp>>, Tp>
    : control(L"BUTTON", Title.data(), BS_GROUPBOX, Rect, true), count(extent<Tp>) {
    initialize(Rect, build<array<stv2, extent<Tp>>>(Texts)); }

  /// constructor for creating a radiobutton which is attached to the specified group
  template<tuple Tp> radiobutton(natt GroupNo, const rect& Rect, const stv2& Title, Tp&& Texts, nat4 BoxStyle = {})
    requires buildable<array<stv2, extent<Tp>>, Tp>
    : control(L"BUTTON", Title.data(), BS_GROUPBOX, Rect, GroupNo, true), count(extent<Tp>) {
    initialize(Rect, build<array<stv2, extent<Tp>>>(Texts)); }

  /// sets the focus to this control
  virtual void setfocus() const override { SetFocus(buttons[state]); }
};


/// class for creating progressbar controls
class yw::progressbar : public yw::control {
public:
  /// default constructor
  progressbar() noexcept = default;

  /// constructor for creating a progressbar which is not attached to any group
  progressbar(const rect& Rect, nat4 Style = {})
    : control(PROGRESS_CLASSW, L"", Style, Rect, false) {
    SendMessageW(hwnd, PBM_SETRANGE, 0, MAKELPARAM(0, 32768)); }

  /// constructor for creating a progressbar which is attached to the specified group
  progressbar(natt GroupNo, const rect& Rect, nat4 Style = {})
    : control(PROGRESS_CLASSW, L"", Style, Rect, GroupNo, false) {
    SendMessageW(hwnd, PBM_SETRANGE, 0, MAKELPARAM(0, 32768)); }

  /// sets the position of this progressbar
  void progress(value Ratio) const { SendMessageW(hwnd, PBM_SETPOS, nat4(Ratio * 32768), 0); }

  /// gets the position of this progressbar
  value progress() const { return SendMessageW(hwnd, PBM_GETPOS, 0, 0) * constant<value{1.0 / 32768}>::value; }
};
