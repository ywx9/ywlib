#pragma once

#include "ywstd.hpp"
#if defined(YWLIB_IMPORT) && YWLIB_IMPORT
#pragma message ("YWLIB_IMPORT")
import ywlib;
#else

#define nat size_t

#define ywlib_func_auto(...) noexcept(noexcept(__VA_ARGS__)) requires requires { __VA_ARGS__; } { return __VA_ARGS__; }
#define ywlib_func_void(...) noexcept(noexcept(__VA_ARGS__)) requires requires { __VA_ARGS__; } { __VA_ARGS__; }
#define ywlib_func_ref(...) noexcept(noexcept(__VA_ARGS__)) -> decltype(auto) requires requires { __VA_ARGS__; } { return __VA_ARGS__; }

export namespace yw { // CORE

using str = std::string;
using str_view = std::string_view;
inline constexpr nat npos = nat(-1);

template<typename T> inline constexpr auto declval = []() noexcept -> T {};

template<typename T, typename... Ts> concept same_as = (std::same_as<T, Ts> && ...);
template<typename T, typename... Ts> concept included_in = (std::same_as<T, Ts> || ...);
template<typename T, typename... Ts> concept different_font = !included_in<T, Ts...>;
template<typename T, typename... Ts> concept derived_from = (std::derived_from<T, Ts> || ...);
template<typename T, typename... Ts> concept castable_to = requires { (static_cast<Ts>(declval<T>()), ...); };
template<typename T, typename... Ts> concept convertible_to = requires { (declval<T(&)(Ts)>()(declval<T>()), ...); };

template<typename T, typename... Ts> concept nt_castable_to =
  castable_to<T, Ts...> && noexcept(noexcept((static_cast<Ts>(declval<T>()), ...)));
template<typename T, typename... Ts> concept nt_convertible_to =
  convertible_to<T, Ts...> && noexcept(noexcept((declval<T(&)(Ts) noexcept>(declval<T>()), ...)));

template<typename T> struct t_type { using type = T; };

template<typename T> struct t_remove_ref : t_type<T> {};
template<typename T> struct t_remove_ref<T&> : t_type<T> {};
template<typename T> struct t_remove_ref<T&&> : t_type<T> {};
template<typename T> using remove_ref = t_remove_ref<T>::type;
template<typename T> using add_lvref = T&;
template<typename T> using add_rvref = remove_ref<T>&&;

inline constexpr auto mv = []<typename T>(T&& Ref) noexcept -> add_rvref<T> { return static_cast<add_rvref<T>>(Ref); };
template<typename T> inline constexpr auto fwd = [](auto&& Ref) noexcept -> T&& { return static_cast<T&&>(Ref); };

template<typename T, typename... As> concept constructible = requires { T{declval<As&&>()...}; };
template<typename T, typename... As> concept nt_constructible = constructible<T, As...> && noexcept(T{declval<As&&>()...});
template<typename T> inline constexpr auto construct = []<typename... As>(As&&... Args)
  noexcept(nt_constructible<T, As...>) requires constructible<T, As...> { return T{fwd<As>(Args)...}; };

template<typename T, typename A> concept assignable = requires { declval<T&&>() = declval<A&&>(); };
template<typename T, typename A> concept nt_assignable = assignable<T, A> && noexcept(declval<T&&>() = declval<A&&>());
inline constexpr auto assign = []<typename T, typename A>(T&& Ref, A&& Val)
  noexcept(nt_assignable<T, A>) requires assignable<T, A> { fwd<T>(Ref) = fwd<T>(Val); };

template<typename T, typename A> concept exchangeable = constructible<T, T> && assignable<T&, A>;
template<typename T, typename A> concept nt_exchangeable = nt_constructible<T, T> && nt_assignable<T&, A>;
inline constexpr auto exchange = []<typename T, typename A>(T& Ref, A&& Val)
  noexcept(nt_exchangeable<T, A>) requires exchangeable<T, A> { T t = mv(Ref); Ref = fwd<A>(Val); return t; };
}

export namespace yw { // RANGE

template<typename It> concept iterator = std::input_or_output_iterator<remove_ref<It>>;
template<typename It> concept input_iterator = std::input_iterator<remove_ref<It>>;
template<typename It> concept forward_iterator = std::forward_iterator<remove_ref<It>>;
template<typename It> concept bidirectional_iterator = std::bidirectional_iterator<remove_ref<It>>;
template<typename It> concept random_access_iterator = std::random_access_iterator<remove_ref<It>>;
template<typename It> concept contiguous_iterator = std::contiguous_iterator<remove_ref<It>>;
template<typename It, typename In> concept output_iterator = std::output_iterator<remove_ref<It>, In>;
template<typename Se, typename It> concept sentinel_for = std::sentinel_for<remove_ref<Se>, remove_ref<It>>;
template<typename Se, typename It> concept sized_sentinel_for = std::sized_sentinel_for<remove_ref<Se>, remove_ref<It>>;

template<typename Rg> concept range = std::ranges::range<Rg>;
template<typename Rg> concept input_range = std::ranges::input_range<Rg>;
template<typename Rg> concept forward_range = std::ranges::forward_range<Rg>;
template<typename Rg> concept bidirectional_range = std::ranges::bidirectional_range<Rg>;
template<typename Rg> concept random_access_range = std::ranges::random_access_range<Rg>;
template<typename Rg> concept contiguous_range = std::ranges::contiguous_range<Rg>;
template<typename Rg> concept view = std::ranges::view<Rg>;
template<typename Rg> concept common_range = std::ranges::common_range<Rg>;
template<typename Rg, typename In> concept output_range = std::ranges::output_range<Rg, In>;

template<range Rg> using iterator_t = std::ranges::iterator_t<Rg>;
template<range Rg> using sentinel_t = std::ranges::sentinel_t<Rg>;

template<typename T> struct t_iter_type {};
template<range Rg> struct t_iter_type<Rg> : t_iter_type<iterator_t<Rg>> {};
template<iterator It> struct t_iter_type<It> {
  using value_t = std::iter_value_t<remove_ref<It>>;
  using differece_t = std::iter_difference_t<remove_ref<It>>;
  using reference_t = std::iter_reference_t<remove_ref<It>>;
  using rvref_t = std::iter_rvalue_reference_t<remove_ref<It>>;
};
template<typename T> requires iterator<T> || range<T> using iter_value_t = t_iter_type<T>::value_t;
template<typename T> requires iterator<T> || range<T> using iter_difference_t = t_iter_type<T>::differece_t;
template<typename T> requires iterator<T> || range<T> using iter_reference_t = t_iter_type<T>::reference_t;
template<typename T> requires iterator<T> || range<T> using iter_rvalue_reference_t = t_iter_type<T>::rvref_t;

inline constexpr auto begin = [](auto&& r) ywlib_func_auto(std::ranges::begin(fwd<decltype(r)>(r)));
inline constexpr auto end = [](auto&& r) ywlib_func_auto(std::ranges::end(fwd<decltype(r)>(r)));
inline constexpr auto empty = [](auto&& r) ywlib_func_auto(std::ranges::empty(fwd<decltype(r)>(r)));
inline constexpr auto size = [](auto&& r) ywlib_func_auto(std::ranges::size(fwd<decltype(r)>(r)));
inline constexpr auto data = [](auto&& r) ywlib_func_auto(std::ranges::data(fwd<decltype(r)>(r)));
inline constexpr auto iter_move = [](auto&& i) ywlib_func_ref(std::ranges::iter_move(fwd<decltype(i)>(i)));
inline constexpr auto iter_swap = [](auto&& i, auto&& j) ywlib_func_void(std::ranges::iter_swap(fwd<decltype(i)>(i), fwd<decltype(j)>(j)));
}

export namespace yw { // ARRAY

/// empty/static/dynamic array
template<typename T, nat N = npos> requires (N >= -1) class array;

/// empty array
template<typename T> class array<T, 0> {
public:
  static constexpr nat count = 0;
  using value_type = T;
  constexpr bool empty() const noexcept { return true; }
  constexpr nat size() const noexcept { return 0; }
  constexpr T* data() noexcept { return nullptr; }
  constexpr const T* data() const noexcept { return nullptr; }
  constexpr T* begin() noexcept { return nullptr; }
  constexpr const T* begin() const noexcept { return nullptr; }
  constexpr T* end() noexcept { return nullptr; }
  constexpr const T* end() const noexcept { return nullptr; }
};

/// static array
template<typename T, nat N> requires (N > 0) class array<T, N> {
public:
  static constexpr nat count = N;
  using value_type = T;
  T _[N]{};
  constexpr bool empty() const noexcept { return false; }
  constexpr nat size() const noexcept { return N; }
  constexpr T* data() noexcept { return _; }
  constexpr const T* data() const noexcept { return _; }
  constexpr T* begin() noexcept { return _; }
  constexpr const T* begin() const noexcept { return _; }
  constexpr T* end() noexcept { return _ + N; }
  constexpr const T* end() const noexcept { return _ + N; }
  constexpr T& operator[](nat I) { return _[I]; }
  constexpr const T& operator[](nat I) const { return _[I]; }
  template<nat I> requires (I < N) constexpr T& get() & noexcept { return _[I]; }
  template<nat I> requires (I < N) constexpr const T& get() const & noexcept { return _[I]; }
  template<nat I> requires (I < N) constexpr T&& get() && noexcept { return mv(_[I]); }
  template<nat I> requires (I < N) constexpr const T&& get() const && noexcept { return mv(_[I]); }
};

/// dynamic array
template<typename T> class array<T, npos> : public std::vector<T> {
  // using base = ;
public:
  using value_type = T;
  constexpr array() noexcept = default;
  constexpr array(array&& a) noexcept : std::vector<T>(static_cast<std::vector<T>&&>(a)) {}
  constexpr array(std::vector<T>&& v) : std::vector<T>(mv(v)) {}
  explicit constexpr array(nat Size) : std::vector<T>(Size) {}
  constexpr array(nat Size, const T& Value) : std::vector<T>(Size, Value) {}
  template<iterator It> requires convertible_to<iter_reference_t<It>, T> constexpr array(It i, It s) : std::vector<T>(i, s) {}
  template<iterator It, sentinel_for<It> Se> requires (!same_as<It, Se> && convertible_to<iter_reference_t<It>, T>)
  constexpr array(It i, Se s) : std::vector<T>(std::common_iterator<It, Se>(i), std::common_iterator<It, Se>(s)) {}
  template<input_range Rg> requires convertible_to<iter_reference_t<Rg>, T>
  constexpr array(Rg&& r) : array(yw::begin(r), yw::end(r)) {}
};

template<typename T, convertible_to<T>... Ts> array(T, Ts...) -> array<T, 1 + sizeof...(Ts)>;
template<typename T> array(nat, const T&) -> array<T, npos>;
template<iterator It, sentinel_for<It> Se> array(It, Se) -> array<iter_value_t<It>, npos>;
template<range Rg> array(Rg&&) -> array<iter_value_t<Rg>, npos>;
}
namespace std {
template<typename T, nat N> requires (N != yw::npos) struct tuple_size<yw::array<T, N>> : integral_constant<size_t, N> {};
template<nat I, typename T, nat N> requires (N != yw::npos) struct tuple_element<I, yw::array<T, N>> : type_identity<T> {};
}

export namespace yw { // STRING

/// literal string
template<nat N> struct literal {

};
}

export namespace yw { // LOGGER

/// logger
class logger {
protected:
  str text{};
public:
  struct level_t {
    str_view name;
    int value;
    friend constexpr bool operator==(const level_t& l, const level_t& r) noexcept { return l.value == r.value; }
    friend constexpr auto operator<=>(const level_t& l, const level_t& r) noexcept { return l.value <=> r.value; }
  };
};
}


export namespace yw {


namespace main {

/// logger for windows/directx system
inline logger log{};
}

class window {
  win::hwnd hwnd{};
  win::hbrush hbrush{};
public:
  ~window() noexcept {
    try {
      if (hbrush && !win::destroy_brush(hbrush)) throw std::runtime_error("failed to destroy brush");
      if (hwnd && !win::destroy_window(hwnd)) throw std::runtime_error("failed to destroy window");
    } catch (const std::exception& e) {
      std::cerr << "window::~window() failed: " << e.what() << std::endl;
    } catch (...) {
      std::cerr << "window::~window() failed" << std::endl;
    }
  }
};


}

#endif
