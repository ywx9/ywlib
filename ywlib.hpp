#pragma once
#include "module/.hpp"

#ifndef nat
#define nat size_t
#endif

// namespace yw {

// /// displays a message box with an OK button; always returns `true`
// inline bool ok(const format_string<cat1> Text, const format_string<cat1> Caption = "OK?") { //
//   return win::ok(nullptr, Text.get().data(), Caption.get().data());
// }

// /// displays a message box with an OK button; always returns `true`
// inline bool ok(const format_string<cat2> Text, const format_string<cat2> Caption = L"OK?") { //
//   return win::ok(nullptr, Text.get().data(), Caption.get().data());
// }

// /// displays a message box with an OK button; always returns `true`
// inline bool ok(stringable auto&& Text, stringable auto&& Caption)    //
//   requires (!(convertible_to<decltype(Text), format_string<cat1>> && //
//               convertible_to<decltype(Caption), format_string<cat1>>) &&
//             !(convertible_to<decltype(Text), format_string<cat2>> && //
//               convertible_to<decltype(Caption), format_string<cat2>>)) {
//   if constexpr (same_as<cat1, iter_value<decltype(Text)>, iter_value<decltype(Caption)>>) {
//     string<cat1> s1(string_view<cat1>(Text)), s2(string_view<cat1>(Caption));
//     return win::ok(nullptr, s1.data(), s2.data());
//   } else {
//     string<cat2> s1(cvt<cat2>(Text)), s2(cvt<cat2>(Caption));
//     return win::ok(nullptr, s1.data(), s2.data());
//   }
// }

// /// displays a message box with an OK button; always returns `true`
// inline bool ok(stringable auto&& Text)                              //
//   requires (!convertible_to<decltype(Text), format_string<cat1>> && //
//             !convertible_to<decltype(Text), format_string<cat2>>) {
//   if constexpr (same_as<cat1, iter_value<decltype(Text)>>) {
//     string<cat1> s(string_view<cat1>(Text));
//     return win::ok(nullptr, s.data(), "OK?");
//   } else {
//     string<cat2> s(cvt<cat2>(Text));
//     return win::ok(nullptr, s.data(), L"OK?");
//   }
// }

// /// displays a message box with Yes and No buttons; returns `true` for Yes and `false` for No
// inline bool yes(const format_string<cat1> Text, const format_string<cat1> Caption = "Yes?") { //
//   return win::yes(nullptr, Text.get().data(), Caption.get().data());
// }

// /// displays a message box with Yes and No buttons; returns `true` for Yes and `false` for No
// inline bool yes(const format_string<cat2> Text, const format_string<cat2> Caption = L"Yes?") { //
//   return win::yes(nullptr, Text.get().data(), Caption.get().data());
// }

// /// displays a message box with Yes and No buttons; returns `true` for Yes and `false` for No
// inline bool yes(stringable auto&& Text, stringable auto&& Caption)   //
//   requires (!(convertible_to<decltype(Text), format_string<cat1>> && //
//               convertible_to<decltype(Caption), format_string<cat1>>) &&
//             !(convertible_to<decltype(Text), format_string<cat2>> && //
//               convertible_to<decltype(Caption), format_string<cat2>>)) {
//   if constexpr (same_as<cat1, iter_value<decltype(Text)>, iter_value<decltype(Caption)>>) {
//     string<cat1> s1(string_view<cat1>(Text)), s2(string_view<cat1>(Caption));
//     return win::yes(nullptr, s1.data(), s2.data());
//   } else {
//     string<cat2> s1(cvt<cat2>(Text)), s2(cvt<cat2>(Caption));
//     return win::yes(nullptr, s1.data(), s2.data());
//   }
// }

// /// displays a message box with Yes and No buttons; returns `true` for Yes and `false` for No
// inline bool yes(stringable auto&& Text)                             //
//   requires (!convertible_to<decltype(Text), format_string<cat1>> && //
//             !convertible_to<decltype(Text), format_string<cat2>>) {
//   if constexpr (same_as<cat1, iter_value<decltype(Text)>>) {
//     string<cat1> s(string_view<cat1>(Text));
//     return win::yes(nullptr, s.data(), "Yes?");
//   } else {
//     string<cat2> s(cvt<cat2>(Text));
//     return win::yes(nullptr, s.data(), L"Yes?");
//   }
// }

// /// stopwatch class to measure time
// class stopwatch {
//   inline static fat8 freq = win::query_performance_frequency();
//   int8 last{};
// public:
//   /// constructs a stopwatch and starts it
//   stopwatch() noexcept { win::query_performance_counter(last); }

//   /// returns elapsed time in seconds
//   fat8 operator()() const noexcept {
//     int8 temp;
//     win::query_performance_counter(temp);
//     return (temp - last) / freq;
//   }

//   operator fat8() const noexcept { return operator()(); }
//   explicit operator fat4() const noexcept { return static_cast<fat4>(operator()()); }

//   /// resets the stopwatch
//   void reset() noexcept { win::query_performance_counter(last); }

//   /// returns elapsed time in seconds and resets the stopwatch
//   fat8 split() noexcept {
//     int8 temp = last;
//     win::query_performance_counter(last);
//     return (last - temp) / freq;
//   }
// };

// namespace main {

// /// instance handle
// inline const win::hinstance hinstance = win::get_module_handle((const wchar_t*)nullptr);

// /// command line arguments
// inline const array<string<cat1>> args = [] {
//   auto a = win::get_command_line();
//   array<string<cat1>> r(a.size());
//   for (nat i = 0; i < a.size(); ++i) r[i] = cvt<cat1>(a[i]);
//   return r;
// }();

// /// logger for main system
// inline logger log{path(args[0]).replace_extension(".log")};

// /// windows username
// inline const string<cat1> username = cvt<cat1>(win::get_username());
// }

// /// window icon
// class wicon {
//   wicon(const wicon&) = delete;
//   wicon& operator=(const wicon&) = delete;
// protected:
//   win::hicon _hicon{};
//   wicon(win::hicon Icon) noexcept : _hicon(Icon) {}
// public:
//   class predefined;
//   wicon(const predefined& PredefinedIcon) = delete;
//   wicon() noexcept = default;
//   wicon(wicon&& i) noexcept : _hicon(exchange(i._hicon, nullptr)) {}
//   operator win::hicon() const noexcept { return _hicon; }
//   explicit operator bool() const noexcept { return _hicon != nullptr; }
//   [[nodiscard]] wicon(const path& File, const source& _ = {}) {
//     try {
//       _hicon = win::create_icon_from_file(File.c_str());
//       if (!_hicon) throw std::runtime_error("failed to create icon from file");
//     } catch (const std::exception& e) {
//       main::log(logger::error, e.what());
//       main::log(logger::error, "failed to create icon", _);
//     }
//   }
//   wicon& operator=(wicon&& Icon) {
//     try {
//       if (_hicon && !win::destroy_icon(_hicon)) main::log(logger::error, "failed to destroy icon");
//       return _hicon = exchange(Icon._hicon, nullptr), *this;
//     } catch (const std::exception& e) {
//       main::log(logger::error, e.what());
//       main::log(logger::error, "failed to move icon");
//     }
//   }
//   ~wicon() noexcept {
//     try {
//       if (_hicon && !win::destroy_icon(_hicon)) main::log(logger::error, "failed to destroy icon");
//     } catch (const std::exception& e) {
//       main::log(logger::error, e.what());
//       main::log(logger::error, "failed to destroy icon");
//     } catch (...) { main::log(logger::error, "failed to destroy icon"); }
//   }
// };
// static_assert(sizeof(wicon) == sizeof(HICON));

// /// predefined window icons
// class wicon::predefined : public wicon {
//   template<int I> predefined(constant<I>) noexcept : wicon(win::load_icon(0, (const wchar_t*)I)) {}
//   predefined(predefined&&) = delete;
//   predefined(const predefined&) = delete;
//   predefined& operator=(predefined&&) = delete;
//   predefined& operator=(const predefined&) = delete;
// public:
//   static const wicon::predefined app, error, question, warning, info, shield;
//   ~predefined() noexcept { _hicon = nullptr; }
// };
// static_assert(sizeof(wicon::predefined) == sizeof(HICON));

// inline const wicon::predefined wicon::predefined::app{constant<32512>{}};
// inline const wicon::predefined wicon::predefined::error{constant<32513>{}};
// inline const wicon::predefined wicon::predefined::question{constant<32514>{}};
// inline const wicon::predefined wicon::predefined::warning{constant<32515>{}};
// inline const wicon::predefined wicon::predefined::info{constant<32516>{}};
// inline const wicon::predefined wicon::predefined::shield{constant<32518>{}};

// /// window cursor
// class wcursor {
//   wcursor(const wcursor&) = delete;
//   wcursor& operator=(const wcursor&) = delete;
// protected:
//   win::hcursor _hcursor{};
//   wcursor(win::hcursor Cursor) noexcept : _hcursor(Cursor) {}
// public:
//   class predefined;
//   wcursor(const predefined& PredefinedCursor) = delete;
//   wcursor() noexcept = default;
//   wcursor(wcursor&& c) noexcept : _hcursor(exchange(c._hcursor, nullptr)) {}
//   operator win::hcursor() const noexcept { return _hcursor; }
//   explicit operator bool() const noexcept { return _hcursor != nullptr; }
//   [[nodiscard]] wcursor(const path& File, const source& _ = {}) {
//     try {
//       _hcursor = win::create_cursor_from_file(File.c_str());
//       if (!_hcursor) throw std::runtime_error("failed to create cursor from file");
//     } catch (const std::exception& e) {
//       main::log(logger::error, e.what());
//       main::log(logger::error, "failed to create cursor", _);
//     }
//   }
//   wcursor& operator=(wcursor&& Cursor) {
//     try {
//       if (_hcursor && !win::destroy_cursor(_hcursor)) main::log(logger::error, "failed to destroy cursor");
//       return _hcursor = exchange(Cursor._hcursor, nullptr), *this;
//     } catch (const std::exception& e) {
//       main::log(logger::error, e.what());
//       main::log(logger::error, "failed to move cursor");
//     }
//   }
//   ~wcursor() noexcept {
//     try {
//       if (_hcursor && !win::destroy_cursor(_hcursor)) main::log(logger::error, "failed to destroy cursor");
//     } catch (const std::exception& e) {
//       main::log(logger::error, e.what());
//       main::log(logger::error, "failed to destroy cursor");
//     } catch (...) { main::log(logger::error, "failed to destroy cursor"); }
//   }
// };
// static_assert(sizeof(wcursor) == sizeof(HCURSOR));

// /// predefined window cursors
// class wcursor::predefined : public wcursor {
//   template<int I> predefined(constant<I>) noexcept : wcursor(win::load_cursor(0, (const wchar_t* I))) {}
//   predefined(predefined&&) = delete;
//   predefined(const predefined&) = delete;
//   predefined& operator=(predefined&&) = delete;
//   predefined& operator=(const predefined&) = delete;
// public:
//   static const wcursor::predefined arrow, ibeam, wait, cross, uparrow, sizeall, sizens, sizewe, no, hand, help;
//   ~predefined() noexcept { _hcursor = nullptr; }
// };
// static_assert(sizeof(wcursor::predefined) == sizeof(HCURSOR));

// inline const wcursor::predefined wcursor::predefined::arrow{constant<32512>{}};
// inline const wcursor::predefined wcursor::predefined::ibeam{constant<32513>{}};
// inline const wcursor::predefined wcursor::predefined::wait{constant<32514>{}};
// inline const wcursor::predefined wcursor::predefined::cross{constant<32515>{}};
// inline const wcursor::predefined wcursor::predefined::uparrow{constant<32516>{}};
// inline const wcursor::predefined wcursor::predefined::sizeall{constant<32646>{}};
// inline const wcursor::predefined wcursor::predefined::sizens{constant<32645>{}};
// inline const wcursor::predefined wcursor::predefined::sizewe{constant<32644>{}};
// inline const wcursor::predefined wcursor::predefined::no{constant<32648>{}};
// inline const wcursor::predefined wcursor::predefined::hand{constant<32649>{}};
// inline const wcursor::predefined wcursor::predefined::help{constant<32651>{}};

// /// window messages
// using wmessage = win::message;
// using enum wmessage;

// /// window
// class window;

// /// window procedure
// template<invocable_r<bool, window&, wmessage, nat8, int8> Fn> //
// struct wprocedure : public Fn {
//   inline static Fn fn{};
//   static int8 CALLBACK wndproc(win::hwnd hwnd, nat4 msg, nat8 wp, int8 lp) noexcept {
//     list<win::hwnd, vector2<int>, win::msg> Dummy{hwnd, {}, {hwnd, wmessage(msg), wp, lp}};
//     if (fn(reinterpret_cast<window&>(Dummy), wmessage(msg), wp, lp)) return 0;
//     else return win::default_procedure(hwnd, msg, wp, lp);
//   }
//   operator win::wndproc() const noexcept { return wndproc; }
// };

// /// window class
// class wclass {
//   wclass(const wclass&) = delete;
//   wclass& operator=(const wclass&) = delete;
// protected:
//   string_view<cat1> _name{};
//   win::hbrush _hbrush{};
//   wclass(format_string<cat1> Name) noexcept : _name(Name.get()) {}
// public:
//   /// window class for controls
//   class control;

//   /// window class styles
//   using style = win::class_style;

//   /// window class name
//   const string_view<cat1>& name = _name;

//   wclass() noexcept = default;
//   wclass(const control&) = delete;
//   wclass(wclass&&) noexcept : _name(exchange(_name, {})), _hbrush(exchange(_hbrush, nullptr)) {}
//   explicit operator bool() const noexcept { return !_name.empty(); }

//   /// creates and registers window class
//   template<typename Fn> [[nodiscard]] wclass(                                        //
//     const format_string<cat1> Name, const wprocedure<Fn>& Proc, wclass::style Style, //
//     const color& Color = color::white, const wicon& Icon = wicon::predefined::app,   //
//     const wcursor& Cursor = wcursor::predefined::arrow, const source& _ = {})
//     : _name(Name.get()), _hbrush(win::create_solid_brush(&Color.r)) {
//     try {
//       bool b = win::register_class(Style, Proc, main::hinstance, Icon, Cursor, _hbrush, name.data());
//       if (!b) throw std::runtime_error("failed to register window class");
//     } catch (const std::exception& e) {
//       main::log(logger::error, e.what());
//       main::log(logger::error, "failed to creates and registers window class", _);
//       if (_hbrush && !win::destroy_brush(exchange(_hbrush, nullptr))) //
//         main::log(logger::error, "failed to delete brush", _);
//       _name = {};
//     }
//   }

//   /// moves window class
//   wclass& operator=(wclass&& wc) noexcept {
//     if (_hbrush && !win::destroy_brush(_hbrush)) //
//       main::log(logger::error, "failed to destroy brush");
//     if (!name.empty() && !win::unregister_class(name.data(), main::hinstance)) //
//       main::log(logger::error, "failed to unregister window class");
//     _name = exchange(wc._name, {});
//     _hbrush = exchange(wc._hbrush, nullptr);
//     return *this;
//   }

//   /// destroys window class
//   ~wclass() noexcept {
//     if (_hbrush && !win::destroy_brush(_hbrush)) //
//       main::log(logger::error, "failed to destroy brush");
//     if (!name.empty() && !win::unregister_class(name.data(), main::hinstance)) //
//       main::log(logger::error, "failed to unregister window class");
//   }
// };
// using enum wclass::style;

// /// window class for controls
// class wclass::control : public wclass {
//   control(const control&) = delete;
//   control& operator=(const control&) = delete;
//   control(control&&) = delete;
//   control& operator=(control&&) = delete;
//   control(format_string<cat1> Name) noexcept : wclass(Name) {}
// public:
//   ~control() noexcept = default;
//   static const wclass::control animation, button, combobox, datetime, edit, header, //
//     hotkey, ipaddress, listbox, listview, calendar, pager, progress, rebar,         //
//     scrollbar, static_, statusbar, tab, toolbar, tooltip, trackbar, treeview, updown;
// };
// inline const wclass::control wclass::control::animation{win::control::animation};
// inline const wclass::control wclass::control::button{win::control::button};
// inline const wclass::control wclass::control::combobox{win::control::combobox};
// inline const wclass::control wclass::control::datetime{win::control::datetime};
// inline const wclass::control wclass::control::edit{win::control::edit};
// inline const wclass::control wclass::control::header{win::control::header};
// inline const wclass::control wclass::control::hotkey{win::control::hotkey};
// inline const wclass::control wclass::control::ipaddress{win::control::ipaddress};
// inline const wclass::control wclass::control::listbox{win::control::listbox};
// inline const wclass::control wclass::control::listview{win::control::listview};
// inline const wclass::control wclass::control::calendar{win::control::calendar};
// inline const wclass::control wclass::control::pager{win::control::pager};
// inline const wclass::control wclass::control::progress{win::control::progress};
// inline const wclass::control wclass::control::rebar{win::control::rebar};
// inline const wclass::control wclass::control::scrollbar{win::control::scrollbar};
// inline const wclass::control wclass::control::static_{win::control::static_};
// inline const wclass::control wclass::control::statusbar{win::control::statusbar};
// inline const wclass::control wclass::control::tab{win::control::tab};
// inline const wclass::control wclass::control::toolbar{win::control::toolbar};
// inline const wclass::control wclass::control::tooltip{win::control::tooltip};
// inline const wclass::control wclass::control::trackbar{win::control::trackbar};
// inline const wclass::control wclass::control::treeview{win::control::treeview};
// inline const wclass::control wclass::control::updown{win::control::updown};

// /// virtual key codes
// using virtual_key = win::virtual_key;
// using enum virtual_key;

// /// window
// class window {
// protected:
//   win::hwnd _hwnd{};
//   vector2<int4> _pad{};
//   window(win::hwnd Hwnd) noexcept : _hwnd(Hwnd) {}
// public:
//   /// window styles
//   using style = win::window_style;

//   /// window message structure
//   struct message_t {
//     win::hwnd hwnd{};
//     wmessage message{};
//     nat8 wparam{};
//     int8 lparam{};
//     nat4 time{};
//     vector2<int> point{};
//     void translate() const noexcept { win::translate_message((const win::msg&)*this); }
//     void dispatch() const noexcept { win::dispatch_message((const win::msg&)*this); }
//   };
//   static_assert(sizeof(message_t) == sizeof(win::msg));

//   /// window message
//   message_t message{};

//   window() noexcept = default;
//   window(window&& w) noexcept : _hwnd(exchange(w._hwnd, nullptr)), _pad(w._pad), message(w.message) {}
//   operator win::hwnd() const noexcept { return _hwnd; }
//   explicit operator bool() const noexcept { return _hwnd != nullptr; }

//   /// creates a window
//   [[nodiscard]] window(                                                                            //
//     const wclass& Class, stringable auto&& Title, style Style, numeric auto&& X, numeric auto&& Y, //
//     numeric auto&& Width, numeric auto&& Height, const source& _ = {}) {
//     try {
//       if (!Class) throw std::runtime_error("invalid window class");
//       const auto title = cvt<cat1>(Title);
//       const bool visible = bool(Style & style::visible);
//       const int size = 500;
//       _hwnd = win::create_window(Style, Class.name.data(), title.data(), int(X), int(Y), //
//                                  int(Width), int(Height), nullptr, nullptr, main::hinstance, nullptr);
//       if (!_hwnd) throw std::runtime_error("failed to call `CreateWindowEx`");
//       [&](vector<int4> r) { win::get_client_rect(_hwnd, &r.x), _pad.x = (size - r.z) / 2, _pad.y = size - r.w - _pad.x; }({});
//       win::set_window_pos(_hwnd, int(X), int(Y), int(Width) + _pad.x * 2, int(Height) + _pad.x + _pad.y);
//       if (visible) win::show_window(_hwnd), win::focus_window(_hwnd);
//     } catch (const std::exception& e) {
//       main::log(logger::error, e.what());
//       main::log(logger::error, "failed to create window", _);
//     }
//   }

//   /// moves window
//   window& operator=(window&& w) {
//     try {
//       if (_hwnd && !win::destroy_window(_hwnd)) throw std::runtime_error("failed to destroy window");
//       _hwnd = exchange(w._hwnd, nullptr), _pad = w._pad, message = w.message;
//     } catch (const std::exception& e) {
//       main::log(logger::error, e.what());
//       main::log(logger::error, "failed to move window");
//     }
//     return *this;
//   }

//   /// destroys window
//   ~window() noexcept {
//     try {
//       if (_hwnd && !win::destroy_window(_hwnd)) main::log(logger::error, "failed to destroy window");
//     } catch (const std::exception& e) {
//       main::log(logger::error, e.what());
//       main::log(logger::error, "failed to destroy window");
//     } catch (...) { main::log(logger::error, "failed to destroy window"); }
//   }

//   /// returns text of the window
//   [[nodiscard]] string<cat1> text() const { return win::get_window_text<cat1>(_hwnd); }

//   /// returns text of the window
//   template<character Ct> [[nodiscard]] string<Ct> text() const { return win::get_window_text<Ct>(_hwnd); }

//   /// sets text of the window
//   void text(stringable auto&& Text) {
//     auto s = cvt<cat2>(Text);
//     return win::set_window_text(_hwnd, s.data());
//   }
// };
