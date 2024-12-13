#pragma once

#include "ywstd.hpp"

#define wchar wchar_t
#define nat size_t

#if defined(YWLIB_IMPORT) && YWLIB_IMPORT
#pragma message ("YWLIB_IMPORT")
import ywlib;
#else

#define ywlib_func_auto(...) noexcept(noexcept(__VA_ARGS__)) requires requires { __VA_ARGS__; } { return __VA_ARGS__; }
#define ywlib_func_void(...) noexcept(noexcept(__VA_ARGS__)) requires requires { __VA_ARGS__; } { __VA_ARGS__; }
#define ywlib_func_ref(...) noexcept(noexcept(__VA_ARGS__)) -> decltype(auto) requires requires { __VA_ARGS__; } { return __VA_ARGS__; }

export namespace yw { // CORE

inline constexpr nat npos = nat(-1);
inline constexpr auto unordered = std::partial_ordering::unordered;
template<typename T> inline constexpr auto declval = []() noexcept -> T {};

template<typename T, typename... Ts> concept same_as = (std::same_as<T, Ts> && ...);
template<typename T, typename... Ts> concept included_in = (std::same_as<T, Ts> || ...);
template<typename T, typename... Ts> concept different_from = !included_in<T, Ts...>;
template<typename T, typename... Ts> concept derived_from = (std::derived_from<T, Ts> || ...);
template<typename T, typename... Ts> concept castable_to = requires { (static_cast<Ts>(declval<T>()), ...); };
template<typename T, typename... Ts> concept convertible_to = requires { (declval<T(&)(Ts)>()(declval<T>()), ...); };

template<typename T, typename... Ts> concept nt_castable_to =
  castable_to<T, Ts...> && noexcept(noexcept((static_cast<Ts>(declval<T>()), ...)));
template<typename T, typename... Ts> concept nt_convertible_to =
  convertible_to<T, Ts...> && noexcept(noexcept((declval<T(&)(Ts) noexcept>(declval<T>()), ...)));
template<typename T> concept numeric =
  nt_castable_to<T, bool, int, long long, unsigned, unsigned long long, float, double>;

struct pass { template<typename T> constexpr T&& operator()(T&& a) const noexcept { return static_cast<T&&>(a); } };

template<auto V, typename T = decltype(V)> requires convertible_to<decltype(V), T> struct constant {
  using type = T;
  static constexpr type value = V;
  consteval operator type() const noexcept { return value; }
  consteval type operator()() const noexcept { return value; }
};

struct none {
  constexpr none() noexcept = default;
  constexpr none(auto&&...) noexcept {}
  constexpr none& operator=(auto&&) noexcept { return *this; }
  explicit constexpr operator bool() const noexcept { return false; }
  constexpr none operator()() const noexcept { return {}; }
  friend constexpr bool operator==(none, none) noexcept { return false; }
  friend constexpr auto operator<=>(none, none) noexcept { return unordered; }
  friend constexpr none operator+(none) noexcept { return {}; }
  friend constexpr none operator-(none) noexcept { return {}; }
  friend constexpr none operator+(none, none) noexcept { return {}; }
  friend constexpr none operator-(none, none) noexcept { return {}; }
  friend constexpr none operator*(none, none) noexcept { return {}; }
  friend constexpr none operator/(none, none) noexcept { return {}; }
  constexpr none& operator+=(none) noexcept { return *this; }
  constexpr none& operator-=(none) noexcept { return *this; }
  constexpr none& operator*=(none) noexcept { return *this; }
  constexpr none& operator/=(none) noexcept { return *this; }
};

struct value {
  double _{};
  constexpr value() noexcept = default;
  constexpr value(none) noexcept : _(std::numeric_limits<double>::quiet_NaN()) {}
  template<numeric T> requires different_from<T, value> constexpr value(T&& v) noexcept : _(static_cast<double>(v)) {}
  template<numeric T> requires different_from<T, value>
  constexpr value& operator=(T&& v) noexcept { return _ = static_cast<double>(v), *this; }
  constexpr operator double&() & noexcept { return _; }
  constexpr operator const double&() const& noexcept { return _; }
  constexpr operator double&&() && noexcept { return static_cast<double&&>(_); }
  constexpr operator const double&&() const&& noexcept { return static_cast<const double&&>(_); }
  template<typename T> requires castable_to<double, T>
  explicit constexpr operator T() const noexcept(nt_castable_to<double, T>) { return static_cast<T>(_); }
  template<numeric T> constexpr value& operator+=(T&& v) noexcept { return _ += static_cast<double>(v), *this; }
  template<numeric T> constexpr value& operator-=(T&& v) noexcept { return _ -= static_cast<double>(v), *this; }
  template<numeric T> constexpr value& operator*=(T&& v) noexcept { return _ *= static_cast<double>(v), *this; }
  template<numeric T> constexpr value& operator/=(T&& v) noexcept { return _ /= static_cast<double>(v), *this; }
};

inline constexpr value e = std::numbers::e_v<double>;
inline constexpr value pi = std::numbers::pi_v<double>;
inline constexpr value phi = std::numbers::phi_v<double>;
inline constexpr value gamma = std::numbers::egamma_v<double>;
inline constexpr value ln2 = std::numbers::ln2_v<double>;
inline constexpr value ln10 = std::numbers::ln10_v<double>;
inline constexpr value log2_10 = 3.3219280948873623478703194294894;
inline constexpr value log10_2 = 0.30102999566398119521373889472449;
inline constexpr value sqrt2 = std::numbers::sqrt2_v<double>;
inline constexpr value sqrt3 = std::numbers::sqrt3_v<double>;
inline constexpr value inf = std::numeric_limits<double>::infinity();
inline constexpr value nan = std::numeric_limits<double>::quiet_NaN();

template<typename T> struct t_type { using type = T; };
template<typename T> concept valid = requires { typename t_type<T>::type; };

template<typename T> struct t_remove_const : t_type<T> {};
template<typename T> struct t_remove_const<const T> : t_type<T> {};
template<typename T> using remove_const = t_remove_const<T>::type;
template<typename T> concept is_const = std::is_const_v<T>;

template<typename T> struct t_remove_volatile : t_type<T> {};
template<typename T> struct t_remove_volatile<volatile T> : t_type<T> {};
template<typename T> using remove_volatile = t_remove_volatile<T>::type;
template<typename T> concept is_volatile = std::is_volatile_v<T>;

template<typename T> using remove_cv = remove_const<remove_volatile<T>>;
template<typename T> concept is_cv = is_const<T> && is_volatile<T>;

template<typename T> struct t_remove_ref : t_type<T> {};
template<typename T> struct t_remove_ref<T&> : t_type<T> {};
template<typename T> struct t_remove_ref<T&&> : t_type<T> {};
template<typename T> using remove_ref = t_remove_ref<T>::type;
template<typename T> using remove_cvref = remove_cv<remove_ref<T>>;
template<typename T> concept is_lvref = std::is_lvalue_reference_v<T>;
template<typename T> concept is_rvref = std::is_rvalue_reference_v<T>;
template<typename T> concept is_reference = is_lvref<T> || is_rvref<T>;

template<typename T> struct t_remove_pointer : t_type<T> {};
template<typename T> struct t_remove_pointer<T*> : t_type<T> {};
template<typename T> struct t_remove_pointer<T* const> : t_type<T> {};
template<typename T> struct t_remove_pointer<T* volatile> : t_type<T> {};
template<typename T> struct t_remove_pointer<T* const volatile> : t_type<T> {};
template<typename T> using remove_pointer = t_remove_pointer<T>::type;
template<typename T> concept is_pointer = std::is_pointer_v<T>;
template<typename T> struct t_remove_all_pointers : t_type<T> {};
template<is_pointer T> struct t_remove_all_pointers<T> : t_remove_all_pointers<remove_pointer<T>> {};
template<typename T> using remove_all_pointers = t_remove_all_pointers<T>::type;

template<typename T> struct t_remove_extent : t_type<T> {};
template<typename T> struct t_remove_extent<T[]> : t_type<T> {};
template<typename T, nat N> struct t_remove_extent<T[N]> : t_type<T> {};
template<typename T> using remove_extent = t_remove_extent<T>::type;
template<typename T> concept is_bounded_array = std::is_bounded_array_v<T>;
template<typename T> concept is_unbounded_array = std::is_unbounded_array_v<T>;
template<typename T> concept is_array = is_bounded_array<T> || is_unbounded_array<T>;
template<typename T> struct t_remove_all_extents : t_type<T> {};
template<is_array T> struct t_remove_all_extents<T> : t_remove_all_extents<remove_extent<T>> {};
template<typename T> using remove_all_extents = t_remove_all_extents<T>::type;

template<typename T> struct t_add_const : t_type<T> {};
template<typename T> struct t_add_const<T&> : t_type<const T&> {};
template<typename T> struct t_add_const<T&&> : t_type<const T&&> {};
template<typename T> using add_const = t_add_const<T>::type;

template<typename T> struct t_add_volatile : t_type<T> {};
template<typename T> struct t_add_volatile<T&> : t_type<volatile T&> {};
template<typename T> struct t_add_volatile<T&&> : t_type<volatile T&&> {};
template<typename T> using add_volatile = t_add_volatile<T>::type;

template<typename T> using add_cv = add_const<add_volatile<T>>;

template<typename T> requires valid<T&> using add_lvref = T&;
template<typename T> requires valid<T&> using add_rvref = remove_ref<T>&&;

template<typename T> struct t_add_pointer : t_type<T> {};
template<typename T> requires valid<T*> struct t_add_pointer<T> : t_type<T*> {};
template<typename T> requires valid<T*> struct t_add_pointer<T&> : t_type<T*&> {};
template<typename T> requires valid<T*> struct t_add_pointer<T&&> : t_type<T*&&> {};
template<typename T> using add_pointer = typename t_add_pointer<T>::type;

template<typename T, nat N> struct t_add_extent : t_type<T> {};
template<typename T, nat N> requires valid<T[N]> struct t_add_extent<T, N> : t_type<T[N]> {};
template<typename T, nat N> requires valid<T[N]> struct t_add_extent<T&, N> : std::add_lvalue_reference<T[N]> {};
template<typename T, nat N> requires valid<T[N]> struct t_add_extent<T&&, N> : std::add_rvalue_reference<T[N]> {};
template<typename T> requires valid<T[]> struct t_add_extent<T, 0> : t_type<T[]> {};
template<typename T> requires valid<T[]> struct t_add_extent<T&, 0> : std::add_lvalue_reference<T[]> {};
template<typename T> requires valid<T[]> struct t_add_extent<T&&, 0> : std::add_rvalue_reference<T[]> {};
template<typename T, nat N> using add_extent = typename t_add_extent<T, N>::type;

template<typename T> concept is_function = std::is_function_v<T>;

template<typename T> struct t_is_member_pointer : constant<false> {};
template<typename M, typename C> struct t_is_member_pointer<M C::*>
  : constant<true> { using class_type = C; using member_type = M; };
template<typename M, typename C> struct t_is_member_pointer<M C::* const> : t_is_member_pointer<M C::*> {};
template<typename M, typename C> struct t_is_member_pointer<M C::* volatile> : t_is_member_pointer<M C::*> {};
template<typename M, typename C> struct t_is_member_pointer<M C::* const volatile> : t_is_member_pointer<M C::*> {};
template<typename T> concept is_member_pointer = t_is_member_pointer<T>::value && is_pointer<T>;
template<is_member_pointer T> using class_type = typename t_is_member_pointer<T>::class_type;
template<is_member_pointer T> using member_type = typename t_is_member_pointer<T>::member_type;
template<typename T> concept is_member_function_pointer = is_member_pointer<T> && is_function<member_type<T>>;
template<typename T> concept is_member_object_pointer = is_member_pointer<T> && !is_member_function_pointer<T>;

template<typename T> concept is_enum = std::is_enum_v<T>;
template<typename T> concept is_scoped_enum = is_enum<T> && !convertible_to<T, bool>;
template<is_enum T> using underlying_type = std::underlying_type_t<T>;
inline constexpr auto to_underlying = [](is_enum auto e) noexcept { return static_cast<underlying_type<decltype(e)>>(e); };


template<typename T> concept is_void = same_as<remove_cv<T>, void>;
template<typename T> concept is_bool = same_as<remove_cv<T>, bool>;
template<typename T> concept is_none = same_as<remove_cv<T>, none>;
template<typename T> concept is_nullptr = same_as<remove_cv<T>, decltype(nullptr)>;
template<typename T> concept character = included_in<remove_cv<T>, char, wchar, char8_t, char16_t, char32_t>;
template<typename T> concept integral = std::integral<T>;
template<typename T> concept signed_integral = std::signed_integral<T>;
template<typename T> concept unsigned_integral = std::unsigned_integral<T>;
template<typename T> concept floating_point = std::floating_point<T>;
template<typename T> concept arithmetic = integral<T> || floating_point<T>;
template<typename T> concept scalar = arithmetic<T> || is_pointer<T> || is_nullptr<T> || is_none<T> || is_enum<T>;

template<typename T> concept is_class = std::is_class_v<T>;
template<typename T> concept is_union = std::is_union_v<T>;
template<typename T> concept is_abstract = is_class<T> && std::is_abstract_v<T>;
template<typename T> concept is_aggregate = std::is_aggregate_v<T>;
template<typename T> concept is_empty = is_class<T> && std::is_empty_v<T>;
template<typename T> concept is_final = is_class<T> && std::is_final_v<T>;
template<typename T> concept is_polymorphic = is_class<T> && std::is_polymorphic_v<T>;
template<typename T> concept has_virtual_destructor = is_class<T> && std::has_virtual_destructor_v<T>;
template<typename T> concept standard_layout = std::is_standard_layout_v<T>;
template<typename T> concept trivially_copyable = std::is_trivially_copyable_v<T>;

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

template<typename F, typename... As> concept invocable = std::invocable<F, As...>;
template<typename F, typename... As> concept nt_invocable = invocable<F, As...> && std::is_nothrow_invocable_v<F, As...>;
template<typename F, typename... As> requires invocable<F, As...> using invoke_result = std::invoke_result_t<F, As...>;
inline constexpr auto invoke = []<typename F, typename... As>(F&& Func, As&&... Args)
  noexcept(nt_invocable<F, As...>) -> decltype(auto) requires invocable<F, As...> {
  if constexpr (is_void<invoke_result<F, As...>>) { std::invoke(fwd<F>(Func), fwd<As>(Args)...); return none{}; }
  else return std::invoke(fwd<F>(Func), fwd<As>(Args)...);
};

template<typename F, typename R, typename... As> concept invocable_r = invocable<F, As...> && convertible_to<invoke_result<F, As...>, R>;
template<typename F, typename R, typename... As> concept nt_invocable_r = nt_invocable<F, As...> && nt_convertible_to<invoke_result<F, As...>, R>;
template<typename R> inline constexpr auto invoke_r = []<typename F, typename... As>(F&& f, As&&... as)
  noexcept(nt_invocable_r<F, R, As...>) -> R requires invocable_r<F, R, As...> {
  return std::invoke(fwd<F>(f), fwd<As>(as)...);
};

template<bool... Bs> inline constexpr nat counts = (Bs + ...);
template<bool... Bs> inline constexpr nat inspects = 0;
template<bool B, bool... Bs> inline constexpr nat inspects<B, Bs...> = B ? 0 : 1 + inspects<Bs...>;
template<convertible_to<nat> auto I, nat N> inline constexpr bool selectable = (is_bool<decltype(I)> && N == 2) || (!is_bool<decltype(I)> && nat(I) < N);

namespace _ {
template<typename T, T I, typename... Ts> struct _select_type : _select_type<nat, I, Ts...> {};
template<bool B, typename Tt, typename Tf> struct _select_type<bool, B, Tt, Tf> : std::conditional<B, Tt, Tf> {};
template<nat I, typename T, typename... Ts> struct _select_type<nat, I, T, Ts...> : _select_type<nat, I - 1, Ts...> {};
template<typename T, typename... Ts> struct _select_type<nat, 0, T, Ts...> : t_type<T> {};
}

/// selects a type from the list
template<convertible_to<nat> auto I, typename... Ts> requires selectable<I, sizeof...(Ts)>
using select_type = typename _::_select_type<decltype(I), I, Ts...>::type;

/// selects a value from the list
template<convertible_to<nat> auto I, auto... Vs> requires selectable<I, sizeof...(Vs)>
inline constexpr auto select_value = select_type<I, constant<Vs>...>::value;

/// selects a parameter from the list
template<convertible_to<nat> auto I> inline constexpr auto select =
  []<typename A, typename... As>(A&& Arg, As&&... Args) noexcept
  -> decltype(auto) requires selectable<I, 1 + sizeof...(As)> {
  if constexpr (is_bool<decltype(I)>) return select<nat(!I)>(fwd<A>(Arg), fwd<As>(Args)...);
  else if constexpr (same_as<decltype(I), nat>) {
    if constexpr (I == 0) return fwd<A>(Arg);
    else return select<I - 1>(fwd<As>(Args)...);
  } else return select<nat(I)>(fwd<A>(Arg), fwd<As>(Args)...);
};

/// adds `const` to `To` if `T` has it.
template<typename T, typename To> using copy_const =
  select_type<is_const<remove_ref<T>>, add_const<To>, To>;

/// adds `volatile` to `To` if `T` has it.
template<typename T, typename To> using copy_volatile =
  select_type<is_volatile<remove_ref<T>>, add_volatile<To>, To>;

/// performs `copy_const` and `copy_volatile`.
template<typename T, typename To> using copy_cv = copy_const<T, copy_volatile<T, To>>;

/// adds reference to `To` if `T` has it; otherwise, removes it.
template<typename T, typename To> using copy_ref =
  select_type<inspects<is_lvref<T>, is_rvref<T>>, add_lvref<To>, add_rvref<To>, remove_ref<To>>;

/// performs `copy_cv` and `copy_ref`.
template<typename T, typename To> using copy_cvref = copy_cv<T, copy_ref<T, To>>;

/// can be implicitly converted through functions
template<typename... Fs> struct caster : public Fs... {
private:
  template<typename T> static constexpr nat i = inspects<same_as<T, invoke_result<Fs>>...>;
  template<typename T> static constexpr nat j = i<T> < sizeof...(Fs) ? i<T> : inspects<convertible_to<T, invoke_result<Fs>>...>;
  template<typename... As> static constexpr nat k = inspects<invocable<Fs, As...>...>;
public:
  using Fs::operator()...;

  template<typename T> requires (j<T> < sizeof...(Fs))
  constexpr operator T() const noexcept(noexcept(select_type<j<T>, Fs...>::operator()()))
  { return select_type<j<T>, Fs...>::operator()(); }

  template<typename... As> requires (k<As...> < sizeof...(Fs))
  constexpr decltype(auto) operator()(As&&... Args) const
    noexcept(noexcept(select_type<k<As...>, Fs...>::operator()(fwd<As>(Args)...)))
  { return select_type<k<As...>, Fs...>::operator()(fwd<As>(Args)...); }
};

template<trivially_copyable T> constexpr auto bitcast = []<trivially_copyable U>(const U& v) noexcept requires (sizeof(T) == sizeof(U)) { return __builtin_bit_cast(T, v); };
inline constexpr auto natcast = []<trivially_copyable T>(const T& v) noexcept {
  if constexpr (sizeof(T) == 1) return bitcast<unsigned char>;
  else if constexpr (sizeof(T) == 2) return bitcast<unsigned short>;
  else if constexpr (sizeof(T) == 4) return bitcast<unsigned int>;
  else if constexpr (sizeof(T) == 8) return bitcast<unsigned long long>;
  else static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);
};
inline constexpr auto popcount = [](const integral auto& i) ywlib_func_auto(std::popcount(natcast(i)));
inline constexpr auto countlz = [](const integral auto& i) ywlib_func_auto(std::countl_zero(natcast(i)));
inline constexpr auto countrz = [](const integral auto& i) ywlib_func_auto(std::countr_zero(natcast(i)));
inline constexpr auto bitfloor = [](const integral auto& i) ywlib_func_auto(std::bit_floor(natcast(i)));
inline constexpr auto bitceil = [](const integral auto& i) ywlib_func_auto(std::bit_ceil(natcast(i)));
}
namespace std {

template<typename T> struct common_type<yw::none, T> : type_identity<yw::none> {};
template<typename T> struct common_type<T, yw::none> : type_identity<yw::none> {};
template<typename Ct> struct formatter<yw::none, Ct> : formatter<basic_string_view<Ct>, Ct> {
  static constexpr Ct _fmt[] = {Ct('n'), Ct('o'), Ct('n'), Ct('e'), Ct('\0')};
  auto format(yw::none, auto& ctx) const { return formatter<basic_string_view<Ct>, Ct>::format(_fmt, ctx); }
};

template<typename T> struct common_type<yw::value, T> : common_type<double, T> {};
template<typename T> struct common_type<T, yw::value> : common_type<T, double> {};
template<typename Ct> struct formatter<yw::value, Ct> : formatter<double, Ct> {
  auto format(yw::value v, auto& ctx) const { return formatter<double, Ct>::format(v._, ctx); }
};
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

using str = std::string;
using wstr = std::wstring;
using str_view = std::string_view;
using wstr_view = std::wstring_view;
using path = std::filesystem::path;
template<character Ct> using string = std::basic_string<Ct>;
template<character Ct> using string_view = std::basic_string_view<Ct>;
template<character Ct, typename... Ts> using format_string = std::basic_format_string<Ct, Ts...>;
template<typename T, typename Ct = iter_value_t<T>> concept stringable = nt_convertible_to<T&, string_view<Ct>>;

template<typename... Ts> inline constexpr str
format(format_string<char, Ts...> Fmt, Ts&&... Args) { return std::format(Fmt, fwd<Ts>(Args)...); }
template<typename... Ts> inline constexpr wstr
format(format_string<wchar, Ts...> Fmt, Ts&&... Args) { return std::format(Fmt, fwd<Ts>(Args)...); }

template<typename... Ts> inline constexpr void
print(format_string<char, Ts...> Fmt, Ts&&... Args) { std::print(Fmt, fwd<Ts>(Args)...); }

template<character Ct> constexpr string<Ct> to_string(const string_view<char> s) {
  if constexpr (same_as<Ct, char32_t>) {
    string<char32_t> r(s.size(), {});
    auto out = r.begin();
    for (auto in = s.begin(), se = s.end(); in < se;) {
      if (unsigned char c = *in++; c < 0x80) *out++ = c;
      else if (c < 0xe0) *out++ = (c & 0x1f) << 6 | (*in++ & 0x3f);
      else if (c < 0xf0) *out++ = (c & 0x0f) << 12 | (*in++ & 0x3f) << 6 | (*in++ & 0x3f);
      else *out++ = (c & 0x07) << 18 | (*in++ & 0x3f) << 12 | (*in++ & 0x3f) << 6 | (*in++ & 0x3f);
    }
    r.resize(out - r.begin());
    return r;
  } else if constexpr (sizeof(Ct) == 2) {
    string<Ct> r(s.size(), {});
    auto out = r.begin();
    for (auto in = s.begin(), se = s.end(); in < se;) {
      if (unsigned char c = *in++; c < 0x80) *out++ = c;
      else if (c < 0xe0) *out++ = (c & 0x1f) << 6 | (*in++ & 0x3f);
      else if (c < 0xf0) *out++ = (c & 0x0f) << 12 | (*in++ & 0x3f) << 6 | (*in++ & 0x3f);
      else if (char32_t u = (c & 0x07) << 18 | (*in++ & 0x3f) << 12 | (*in++ & 0x3f) << 6 | (*in++ & 0x3f); (u -= 0x10000), true)
        *out++ = Ct(0xd800 | (u >> 10)), *out++ = Ct(0xdc00 | (u & 0x3ff));
    }
    r.resize(out - r.begin());
    return r;
  } else return string<Ct>(bitcast<string_view<Ct>>(s));
}

template<character Ct> constexpr string_view<Ct> to_string(const string_view<wchar> s) {
  if constexpr (same_as<Ct, char32_t>) { // utf-16 to utf-32
    string<char32_t> r(s.size(), {});
    auto out = r.begin();
    for (auto in = s.begin(), se = s.end(); in < se;) {
      if (wchar c = *in++; c < 0xd800 || c >= 0xe000) *out++ = c;
      else if (c < 0xdc00) {
        if (wchar d = *in++; d >= 0xdc00 && d < 0xe000) *out++ = (c & 0x3ff) << 10 | (d & 0x3ff) | 0x10000;
        else *out++ = c;
      } else *out++ = c;
    }
  }
}
}

export namespace yw { // FILE

namespace file {

inline bool exists(const path& Path) noexcept { return std::filesystem::exists(Path); }
inline bool is_file(const path& Path) noexcept { return std::filesystem::is_regular_file(Path); }
inline bool is_dir(const path& Path) noexcept { return std::filesystem::is_directory(Path); }
inline bool is_empty(const path& Path) noexcept { return std::filesystem::is_empty(Path); }
inline nat size(const path& Path) noexcept { return std::filesystem::file_size(Path); }
inline void remove(const path& Path) { std::filesystem::remove(Path); }
inline void rename(const path& Old, const path& New) { std::filesystem::rename(Old, New); }
inline void copy(const path& From, const path& To) { std::filesystem::copy(From, To); }
inline bool create_dir(const path& Path) { return std::filesystem::create_directories(Path); }
inline bool read(const path& Path, void* Data, nat Bytes) {
  if (auto ifs = std::ifstream(Path, std::ios::binary); ifs) {
    ifs.read(static_cast<char*>(Data), Bytes);
    return ifs.good();
  } else return false;
}
template<contiguous_range Rg> bool read(const path& Path, Rg&& r) {
  return read(Path, yw::data(r), yw::size(r) * sizeof(iter_value_t<Rg>));
}
inline str read(const path& Path) {
  str s(size(Path), '\0');
  if (read(Path, s.data(), s.size())) return s;
  else throw std::runtime_error("failed to read file");
}
inline bool write(const path& Path, const void* Data, nat Bytes) {
  if (auto ofs = std::ofstream(Path, std::ios::binary); ofs) {
    ofs.write(static_cast<const char*>(Data), Bytes);
    return ofs.good();
  } else return false;
}
template<contiguous_range Rg> bool write(const path& Path, Rg&& r) {
  return write(Path, yw::data(r), yw::size(r) * sizeof(iter_value_t<Rg>));
}
}
}

export namespace yw { // SOURCE

struct source {
  explicit source(const char*) = delete;
  const char* const file;
  const char* const func;
  const unsigned line, column;

  consteval source(
    const char* File = __builtin_FILE(),
    const char* Func = __builtin_FUNCTION(),
    unsigned Line = __builtin_LINE(),
    unsigned Column = __builtin_COLUMN()) noexcept
    : file(File), func(Func), line(Line), column(Column) {}

  /// returns a string; `{file}({line},{column})`
  template<character Ct> string<Ct> to_string() const {
    string_view<Ct> f(file);
    string<Ct> s0{}, s1 = to_string<Ct>(line), s2 = to_string<Ct>(column);
    s0.reserve(f.size() + s1.size() + s2.size() + 3);
    s0.append(f), s0.push_back(Ct('('));
    s0.append(s1), s0.push_back(Ct(','));
    s0.append(s2), s0.push_back(Ct(')'));
    return s0;
  }

  /// returns a string; `{file}({line},{column})`
  str to_string() const { return to_string<char>(); }
};
}
namespace std {

template<typename Ct> struct formatter<yw::source, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::source& s, auto& ctx) const {
    return formatter<string<Ct>, Ct>::format(s.to_string<Ct>(), ctx);
  }
};
template<typename Ct, typename Tr> std::basic_ostream<Ct, Tr>
operator<<(std::basic_ostream<Ct, Tr>& os, const yw::source& s) { return os << s.to_string<Ct>(); }
}

export namespace yw { // TIME

namespace _ {
auto get_zoned_time() { return std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now()); }
}

/// date
struct date {
  int year{}, month{}, day{};

  /// default constructor; gets the current date
  date() : date(_::get_zoned_time().get_local_time()) {}

  /// constructor with year, month, and day
  date(numeric auto&& Year, numeric auto&& Month, numeric auto&& Day) noexcept
    : year(int(Year)), month(int(Month)), day(int(Day)) {}

  /// constructor from a `time_point`
  template<typename Clock, typename Duration> date(const std::chrono::time_point<Clock, Duration>& tp) {
    const auto ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(tp));
    year = int(ymd.year()), month = int(nat4(ymd.month())), day = int(nat4(ymd.day()));
  }

  /// returns a string; `YYYY-MM-DD`
  template<character Ct> string<Ct> to_string() const {
    string<Ct> s(10, {});
    if constexpr (sizeof(Ct) == 4) {
      auto t = std::format("{:04d}-{:02d}-{:02d}", year, month, day);
      std::ranges::copy(t, s.data());
    } else if constexpr (sizeof(Ct) == 1) std::format_to(s.data(), "{:04d}-{:02d}-{:02d}", year, month, day);
    else if constexpr (sizeof(Ct) == 2) std::format_to(s.data(), L"{:04d}-{:02d}-{:02d}", year, month, day);
    return s;
  }

  /// returns a string; `YYYY-MM-DD`
  str to_string() const { return to_string<char>(); }
};

/// clock
struct clock {
  int hour{}, minute{}, second{};

  /// default constructor; gets the current time
  clock() : clock(_::get_zoned_time().get_local_time()) {}

  /// constructor with hour, minute, and second
  clock(numeric auto&& Hour, numeric auto&& Minute, numeric auto&& Second) noexcept
    : hour(int(Hour)), minute(int(Minute)), second(int(Second)) {}

  /// constructor from a `time_point`
  template<typename Clock, typename Duration> clock(const std::chrono::time_point<Clock, Duration>& tp) {
    const std::chrono::hh_mm_ss hms(std::chrono::floor<std::chrono::seconds>(tp - std::chrono::floor<std::chrono::days>(tp)));
    hour = int(hms.hours().count()), minute = int(hms.minutes().count()), second = int(hms.seconds().count());
  }

  /// returns a string; `HH:MM:SS`
  template<character Ct> string<Ct> to_string() const {
    string<Ct> s(8, {});
    if constexpr (sizeof(Ct) == 4) {
      auto t = std::format("{:02d}:{:02d}:{:02d}", hour, minute, second);
      std::ranges::copy(t, s.data());
    } else if constexpr (sizeof(Ct) == 1) std::format_to(s.data(), "{:02d}:{:02d}:{:02d}", hour, minute, second);
    else if constexpr (sizeof(Ct) == 2) std::format_to(s.data(), L"{:02d}:{:02d}:{:02d}", hour, minute, second);
    return s;
  }

  /// returns a string; `HH:MM:SS`
  str to_string() const { return to_string<char>(); }
};

/// date and clock
struct time {
  template<character Ct> static constexpr Ct format[] = { // {:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}
    Ct('{'), Ct(':'), Ct('0'), Ct('4'), Ct('d'), Ct('}'), Ct('-'), Ct('{'), Ct(':'), Ct('0'), Ct('2'), Ct('d'), Ct('}'),
    Ct('-'), Ct('{'), Ct(':'), Ct('0'), Ct('2'), Ct('d'), Ct('}'), Ct(' '), Ct('{'), Ct(':'), Ct('0'), Ct('2'), Ct('d'),
    Ct('}'), Ct(':'), Ct('{'), Ct(':'), Ct('0'), Ct('2'), Ct('d'), Ct('}'), Ct('\0')};

  yw::date date;
  yw::clock clock;

  /// default constructor; gets the current date and time
  time() : time(_::get_zoned_time().get_local_time()) {}

  /// constructor with date and clock
  time(const yw::date& Date, const yw::clock& Clock) noexcept : date(Date), clock(Clock) {}

  /// constructor from a `time_point`
  template<typename Clock, typename Duration>
  time(const std::chrono::time_point<Clock, Duration>& tp) : date(tp), clock(tp) {}

  operator yw::date() const { return date; }
  operator yw::clock() const { return clock; }

  /// returns a string; `YYYY-MM-DD HH:MM:SS`
  template<character Ct> string<Ct> to_string() const {
    string<Ct> s(19, {});
    if constexpr (sizeof(Ct) == 4) {
      auto t = std::format(format<char>, date.year, date.month, date.day, clock.hour, clock.minute, clock.second);
      std::ranges::copy(t, s.data());
    } else if constexpr (sizeof(Ct) == 1)
      std::format_to(s.data(), format<char>, date.year, date.month, date.day, clock.hour, clock.minute, clock.second);
    else if constexpr (sizeof(Ct) == 2)
      std::format_to(s.data(), format<wchar>, date.year, date.month, date.day, clock.hour, clock.minute, clock.second);
    return s;
  }

  /// returns a string; `YYYY-MM-DD HH:MM:SS`
  str to_string() const { return to_string<char>(); }
};

/// returns the current time
inline constexpr caster now{[] { return time{}; }};
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
  static constexpr level_t all{"all", 0};
  static constexpr level_t debug{"debug", 10};
  static constexpr level_t info{"info", 20};
  static constexpr level_t warn{"warn", 30};
  static constexpr level_t error{"error", 40};
  static constexpr level_t fatal{"fatal", 50};

  yw::path path{};
  level_t level{info};
  bool console{true};

  logger() noexcept = default;
  logger(const yw::path& Path, level_t Level = info, bool Console = true) noexcept
    : path(Path), level(Level), console(Console) {}
  ~logger() noexcept {
    if (text.empty() || path.empty()) return;
    try {
      if (!file::exists(path)) throw std::runtime_error("file not found");
      else if (!file::write(path, text)) throw std::runtime_error("failed to write file");
    } catch (const std::exception& e) { std::cerr << "logger::~logger() failed: " << e.what() << std::endl;
    } catch (...) { std::cerr << "logger::~logger() failed" << std::endl; }
  }

  /// appends text to the log; `{time} [{level}] {text}`
  void operator()(level_t Level, stringable auto&& Text) noexcept {
    if (level < Level) return;
    try {
      str s;
      if constexpr (same_as<iter_value_t<decltype(Text)>, char>)
        s = format("{} [{}] {}\n", now(), Level.name, str_view(Text));
      else s = format("{} [{}] {}\n", now(), Level.name, cvt<char>(Text));
      if (console) std::cout << s;
      text += s;
    } catch (const std::exception& e) { std::cerr << "logger::operator() failed: " << e.what() << std::endl;
    } catch (...) { std::cerr << "logger::operator() failed" << std::endl; }
  }

  /// appends text to the log; `{time} [{level}] {text}`
  void operator()(level_t Level, stringable auto&& Text, const source& _) noexcept {
    if (level < Level) return;
    try {
      str s;
      if constexpr (same_as<iter_value_t<decltype(Text)>, char>)
        s = format("{} [{}] {}\n", now(), Level.name, str_view(Text));
      else s = format("{} [{}] {}\n", now(), Level.name, cvt<char>(Text));
      if (console) std::cout << _ << std::endl << s;
      text += s;
    } catch (const std::exception& e) { std::cerr << "logger::operator() failed: " << e.what() << std::endl;
    } catch (...) { std::cerr << "logger::operator() failed" << std::endl; }
  }

  /// appends formatted text to the log; `{time} [{level}] {text}`
  // void operator()(level_t Level, const format_string<)
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
