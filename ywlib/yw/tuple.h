#pragma once
#include "yw/core.h"

inline constexpr char yw_tuple[] = "yw/tuple.h";

namespace yw {

/// MARK: sequence

template<auto... Vs> struct sequence;

namespace _ {
template<typename T, typename U> struct _to_sequence : std::type_identity<void> {};
template<template<auto...> typename Tm, auto... Vs, typename U> struct _to_sequence<Tm<Vs...>, U>
  : std::type_identity<sequence<static_cast<U>(Vs)...>> {};
template<template<typename T, T...> typename Tm, typename T, T... Vs, typename U> struct _to_sequence<Tm<T, Vs...>, U>
  : std::type_identity<sequence<static_cast<U>(Vs)...>> {};
template<template<typename T, T...> typename Tm, typename T, T... Vs> struct _to_sequence<Tm<T, Vs...>, none>
  : std::type_identity<sequence<Vs...>> {};
template<template<auto...> typename Tm, auto... Vs> struct _to_sequence<Tm<Vs...>, none>
  : std::type_identity<sequence<Vs...>> {};
} // namespace _

template<typename T, typename U = none> using to_sequence = _::_to_sequence<T, U>::type;
template<typename T, typename U = none> concept is_sequence = !is_void<to_sequence<T, U>>;

namespace _ {
template<typename Sq, size_t N> inline constexpr bool _indices_for{false};
template<size_t... Is, size_t N> inline constexpr bool _indices_for<sequence<Is...>, N>{(sizeof...(Is) < N)};
} // namespace _

template<typename Sq, typename Tp> concept indices_for = _::_indices_for<to_sequence<Sq, size_t>, extent<Tp>>;

namespace _ {
template<size_t I, size_t N, auto P, auto... Vs> struct _make_sequence : _make_sequence<I + 1, N, P, Vs..., P(I)> {};
template<size_t N, auto P, auto... Vs> struct _make_sequence<N, N, P, Vs...> : std::type_identity<sequence<Vs...>> {};
} // namespace _

template<size_t Begin, size_t End, auto Proj = pass{}> requires(Begin <= End) && invocable<decltype(Proj), size_t>
using make_sequence = _::_make_sequence<Begin, End, Proj>::type;
template<typename T> using make_indices_for = make_sequence<0, extent<T>>;

namespace _ {
template<typename S, size_t... Is> struct _extracting_indices : std::type_identity<void> {};
template<bool... Bs> struct _extracting_indices<sequence<Bs...>>
  : _extracting_indices<sequence<Bs...>, 0, sizeof...(Bs)> {};
template<bool... Bs, size_t I, size_t N, size_t... Is> struct _extracting_indices<sequence<Bs...>, I, N, Is...>
  : select_type<
      select_value<I, Bs...>, _extracting_indices<sequence<Bs...>, I + 1, N, Is..., I>,
      _extracting_indices<sequence<Bs...>, I + 1, N, Is...>> {};
template<bool... Bs, size_t N, size_t... Is> struct _extracting_indices<sequence<Bs...>, N, N, Is...>
  : std::type_identity<sequence<Is...>> {};
} // namespace _

template<is_sequence<bool> S> using extracting_indices = _::_extracting_indices<to_sequence<S, bool>>::type;

namespace _ {
template<typename S, typename T> struct _sequence_extract : std::type_identity<void> {};
template<auto... Vs, size_t... Is> struct _sequence_extract<sequence<Vs...>, sequence<Is...>>
  : std::type_identity<sequence<select_value<Is, Vs...>...>> {};
template<typename S, typename T> struct _sequence_append : std::type_identity<void> {};
template<auto... Vs, auto... Ws> struct _sequence_append<sequence<Vs...>, sequence<Ws...>>
  : std::type_identity<sequence<Vs..., Ws...>> {};
} // namespace _

template<auto... Vs> struct sequence {
  static constexpr size_t count{sizeof...(Vs)};
  template<size_t I> requires(I < sizeof...(Vs)) static constexpr auto at = select_value<I, Vs...>;
  template<size_t I> requires(I < sizeof...(Vs)) using type_at = select_type<I, decltype(Vs)...>;
  template<indices_for<sequence> Ix> using extract = _::_sequence_extract<sequence, Ix>;
  template<size_t N> requires(N <= sizeof...(Vs)) using fore = extract<make_sequence<0, N>>;
  template<size_t N> requires(N <= sizeof...(Vs)) using back = extract<make_sequence<sizeof...(Vs) - N, sizeof...(Vs)>>;
  template<is_sequence Sq> using append = _::_sequence_append<sequence, to_sequence<Sq>>::type;
  template<size_t I> requires(I < sizeof...(Vs)) using remove = fore<I>::template append<back<sizeof...(Vs) - I - 1>>;
  template<size_t I, is_sequence Sq> requires(I <= sizeof...(Vs))
  using insert = typename fore<I>::template append<Sq>::template append<back<sizeof...(Vs) - I>>;
  template<template<auto...> typename Tm> using expand = Tm<Vs...>;
  template<size_t I> requires(I < sizeof...(Vs)) constexpr const auto&& get() const noexcept {
    return std::move(at<I>);
  }
};

/// MARK: typepack

template<typename... Ts> struct typepack;

namespace _ {
template<typename T, typename S> struct _to_typepack : std::type_identity<void> {};
template<typename T, size_t... Is> struct _to_typepack<T, sequence<Is...>>
  : std::type_identity<typepack<element_t<T, Is>...>> {};
} // namespace _

template<typename T> using to_typepack = _::_to_typepack<T, make_indices_for<T>>::type;

namespace _ {
template<typename T, typename Is> struct _typepack_extract : std::type_identity<void> {};
template<typename... Ts, size_t... Is> struct _typepack_extract<typepack<Ts...>, sequence<Is...>>
  : std::type_identity<typepack<select_type<Is, Ts...>...>> {};
template<typename T, typename U> struct _typepack_append : std::type_identity<void> {};
template<typename... Ts, typename... Us> struct _typepack_append<typepack<Ts...>, typepack<Us...>>
  : std::type_identity<typepack<Ts..., Us...>> {};
} // namespace _

template<typename... Ts> struct typepack {
  static constexpr size_t count{sizeof...(Ts)};
  template<size_t I> requires(I < sizeof...(Ts)) using at = select_type<I, Ts...>;
  template<indices_for<typepack> Ix> using extract = _::_typepack_extract<typepack, to_sequence<Ix, size_t>>::type;
  template<size_t N> requires(N <= sizeof...(Ts)) using fore = extract<make_sequence<0, N>>;
  template<size_t N> requires(N <= sizeof...(Ts)) using back = extract<make_sequence<sizeof...(Ts) - N, sizeof...(Ts)>>;
  template<specialization_of<yw::typepack> T> using append = _::_typepack_append<typepack, T>;
  template<size_t I> requires(I < sizeof...(Ts)) using remove = fore<I>::template append<back<sizeof...(Ts) - I - 1>>;
  template<size_t I, specialization_of<yw::typepack> T> requires(I <= sizeof...(Ts))
  using insert = typename fore<I>::template append<T>::template append<back<sizeof...(Ts) - I>>;
  template<template<typename...> typename Tm> using expand = Tm<Ts...>;
  template<size_t I> requires(I < sizeof...(Ts)) constexpr const at<I> get() const noexcept;
};

/// MARK: apply

inline constexpr struct {
  struct internal {
    template<typename F, typename Tp, size_t... Is>
    static constexpr decltype(auto) operator()(F&& f, Tp&& t, sequence<Is...>) noexcept(
      noexcept(invoke(static_cast<F&&>(f), yw::get<Is>(static_cast<Tp&&>(t))...)))
      requires invocable<F, decltype(yw::get<Is>(static_cast<Tp&&>(t)))...> {
      return invoke(static_cast<F&&>(f), yw::get<Is>(static_cast<Tp&&>(t))...);
    }
  };
  template<typename F, typename Tp> static constexpr decltype(auto) operator()(F&& f, Tp&& Tuple) noexcept(
    noexcept(internal()(static_cast<F&&>(f), static_cast<Tp&&>(Tuple), make_indices_for<Tp>{})))
    requires requires { internal()(static_cast<F&&>(f), static_cast<Tp&&>(Tuple), make_indices_for<Tp>{}); } {
    return internal()(static_cast<F&&>(f), static_cast<Tp&&>(Tuple), make_indices_for<Tp>{});
  }
} apply;

template<typename F, typename Tp> concept applyable = requires { apply(std::declval<F>(), std::declval<Tp>()); };
template<typename F, typename Tp> concept nt_applyable =
  applyable<F, Tp> && noexcept(apply(std::declval<F>(), std::declval<Tp>()));
template<typename F, typename Tp> using apply_result = decltype(apply(std::declval<F>(), std::declval<Tp>()));

/// MARK: build

template<typename T, typename Tp> concept buildable = applyable<decltype(construct<T>), Tp>;
template<typename T, typename Tp> concept nt_buildable = nt_applyable<decltype(construct<T>), Tp>;
template<typename T> inline constexpr auto build = []<typename Tp>(Tp&& Tuple) noexcept(nt_buildable<T, Tp>) -> T
  requires buildable<T, Tp> { return apply(construct<T>, static_cast<Tp&&>(Tuple)); };

/// MARK: vapply

inline constexpr struct {
  struct internal {
    template<size_t I, size_t N, typename F, typename... Ts> requires(I == N)
    static constexpr void operator()(F&, Ts&&...) noexcept {}
    template<size_t I, size_t N, typename F, typename T, typename... Ts> requires(I < N)
    static constexpr void operator()(F&& f, T&& t, Ts&&... ts) noexcept(
      nt_invocable<F&, element_t<T, I>, element_t<Ts, I>...> &&
      noexcept(operator()<I + 1, N>(f, static_cast<T&&>(t), static_cast<Ts&&>(ts)...))) {
      invoke(f, get<I>(static_cast<T&&>(t)), get<I>(static_cast<Ts&&>(ts))...);
      operator()<I + 1, N>(f, static_cast<T&&>(t), static_cast<Ts&&>(ts)...);
    }
  };
  template<invocable F> static constexpr void operator()(F&& f) noexcept(nt_invocable<F>) {
    invoke(static_cast<F&&>(f));
  }
  template<typename F, typename T, typename... Ts> requires((extent<T> == extent<Ts>) && ...)
  static constexpr void operator()(F&& f, T&& t, Ts&&... ts) noexcept(
    noexcept(internal::operator()<0, extent<T>>(f, std::declval<T&&>(), std::declval<Ts&&>()...)))
    requires requires { internal::operator()<0, extent<T>>(f, std::declval<T&&>(), std::declval<Ts&&>()...); } {
    internal::operator()<0, extent<T>>(f, static_cast<T&&>(t), static_cast<Ts&&>(ts)...);
  }
} vapply;

template<typename F, typename... Ts> concept vapplyable =
  requires { vapply(std::declval<F>(), std::declval<Ts>()...); };
template<typename F, typename... Ts> concept nt_vapplyable =
  vapplyable<F, Ts...> && noexcept(vapply(std::declval<F>(), std::declval<Ts>()...));

/// MARK: vassign

template<typename T, typename U> concept vassignable = vapplyable<decltype(assign), T, U>;
template<typename T, typename U> concept nt_vassignable = nt_vapplyable<decltype(assign), T, U>;
inline constexpr auto vassign = []<typename T, typename U>(T&& t, U&& u) noexcept(nt_vassignable<T, U>) -> void
  requires vassignable<T, U> { vapply(assign, static_cast<T&&>(t), static_cast<U&&>(u)); };

/// MARK: tuple

template<typename... Ts> struct tuple;
template<typename... Ts> using tuple_base = typepack<Ts...>::template fore<sizeof...(Ts) - 1>::template expand<tuple>;

namespace _ {
template<typename T, typename U, typename V> struct _tuple_from_typepack;
template<typename U, typename V> struct _tuple_from_typepack<typepack<>, U, V> : std::type_identity<tuple<>> {};
template<typename... Ts, typename U, template<typename...> typename Tm, typename... Vs>
struct _tuple_from_typepack<typepack<Ts...>, U, Tm<Vs...>> : std::type_identity<tuple<copy_cvref<U, Tm<Ts>>...>> {};
template<typename... Ts, typename U, template<typename, auto...> typename Tm, typename V, auto... Vs>
struct _tuple_from_typepack<typepack<Ts...>, U, Tm<V, Vs...>>
  : std::type_identity<tuple<copy_cvref<U, Tm<Ts, Vs...>>...>> {};
template<typename... Ts, typename U, typename V> struct _tuple_from_typepack<typepack<Ts...>, U, V>
  : std::type_identity<tuple<copy_cvref<U, Ts>...>> {};
} // namespace _

template<typename... Ts> struct tuple : tuple_base<Ts...> {
  static constexpr size_t count{sizeof...(Ts)};
  using last_type = select_type<sizeof...(Ts) - 1, Ts...>;
  select_type<sizeof...(Ts) - 1, Ts...> last;
  template<size_t I> requires(I < sizeof...(Ts)) constexpr auto get() & noexcept -> select_type<I, Ts...>& {
    if constexpr (I == sizeof...(Ts) - 1) return last;
    else return tuple_base<Ts...>::template get<I>();
  }
  template<size_t I> requires(I < sizeof...(Ts)) constexpr auto get() const& noexcept -> const select_type<I, Ts...>& {
    if constexpr (I == sizeof...(Ts) - 1) return last;
    else return tuple_base<Ts...>::template get<I>();
  }
  template<size_t I> requires(I < sizeof...(Ts)) constexpr auto get() && noexcept -> select_type<I, Ts...>&& {
    if constexpr (I == sizeof...(Ts) - 1) return static_cast<select_type<I, Ts...>&&>(last);
    else return static_cast<tuple_base<Ts...>&&>(*this).template get<I>();
  }
  template<size_t I> requires(I < sizeof...(Ts))
  constexpr auto get() const&& noexcept -> const select_type<I, Ts...>&& {
    if constexpr (I == sizeof...(Ts) - 1) return static_cast<const select_type<I, Ts...>&&>(last);
    else return static_cast<const tuple_base<Ts...>&&>(*this).template get<I>();
  }
  template<typename A> constexpr tuple& operator=(A&& Arg) & requires vassignable<tuple&, A> {
    return vassign(*this, static_cast<A&&>(Arg)), *this;
  }
  template<typename A> constexpr const tuple& operator=(A&& Arg) const& requires vassignable<const tuple&, A> {
    return vassign(*this, static_cast<A&>(Arg)), *this;
  }
  template<typename A> constexpr tuple&& operator=(A&& Arg) && requires vassignable<tuple&&, A> {
    return vassign(static_cast<tuple&&>(*this), static_cast<A&&>(Arg)), static_cast<tuple&&>(*this);
  }
  template<typename A> constexpr const tuple&& operator=(A&& Arg) const&& requires vassignable<const tuple&&, A> {
    return vassign(static_cast<const tuple&&>(*this), static_cast<A&&>(Arg)), static_cast<const tuple&&>(*this);
  }
};

template<typename T1, typename T2, typename T3> struct tuple<T1, T2, T3> : tuple<T1, T2> {
  using tuple<T1, T2>::first;
  using tuple<T1, T2>::second;
  static constexpr size_t count = 3;
  using third_type = T3;
  third_type third;
  template<size_t I> requires(I < 3) constexpr auto get() & noexcept -> select_type<I, T1, T2, T3>& {
    return select<I>(first, second, third);
  }
  template<size_t I> requires(I < 3) constexpr auto get() const& noexcept -> const select_type<I, T1, T2, T3>& {
    return select<I>(first, second, third);
  }
  template<size_t I> requires(I < 3) constexpr auto get() && noexcept -> select_type<I, T1, T2, T3>&& {
    return static_cast<select_type<I, T1, T2, T3>&&>(select<I>(first, second, third));
  }
  template<size_t I> requires(I < 3) constexpr auto get() const&& noexcept -> const select_type<I, T1, T2, T3>&& {
    return static_cast<const select_type<I, T1, T2, T3>&&>(select<I>(first, second, third));
  }
  template<typename A> constexpr tuple& operator=(A&& Arg) & requires vassignable<tuple&, A> {
    return vassign(*this, static_cast<A&&>(Arg)), *this;
  }
  template<typename A> constexpr const tuple& operator=(A&& Arg) const& requires vassignable<const tuple&, A> {
    return vassign(*this, static_cast<A&&>(Arg)), *this;
  }
  template<typename A> constexpr tuple&& operator=(A&& Arg) && requires vassignable<tuple&&, A> {
    return vassign(static_cast<tuple&&>(*this), static_cast<A&&>(Arg)), static_cast<tuple&&>(*this);
  }
  template<typename A> constexpr const tuple&& operator=(A&& Arg) const&& requires vassignable<const tuple&&, A> {
    return vassign(static_cast<const tuple&&>(*this), static_cast<A&&>(Arg)), static_cast<const tuple&&>(*this);
  }
};

template<typename T1, typename T2> struct tuple<T1, T2> : tuple<T1> {
  using tuple<T1>::first;
  static constexpr size_t count = 2;
  using second_type = T2;
  second_type second;
  template<size_t I> requires(I < 2) constexpr auto get() & noexcept -> select_type<I, T1, T2>& {
    return select<I>(first, second);
  }
  template<size_t I> requires(I < 2) constexpr auto get() const& noexcept -> const select_type<I, T1, T2>& {
    return select<I>(first, second);
  }
  template<size_t I> requires(I < 2) constexpr auto get() && noexcept -> select_type<I, T1, T2>&& {
    return static_cast<select_type<I, T1, T2>&&>(select<I>(first, second));
  }
  template<size_t I> requires(I < 2) constexpr auto get() const&& noexcept -> const select_type<I, T1, T2>&& {
    return static_cast<const select_type<I, T1, T2>&&>(select<I>(first, second));
  }
  template<typename A> constexpr tuple& operator=(A&& Arg) & requires vassignable<tuple&, A> {
    return vassign(*this, static_cast<A&&>(Arg)), *this;
  }
  template<typename A> constexpr const tuple& operator=(A&& Arg) const requires vassignable<const tuple&, A> {
    return vassign(*this, static_cast<A&&>(Arg)), *this;
  }
  template<typename A> constexpr tuple&& operator=(A&& Arg) && requires vassignable<tuple&&, A> {
    return vassign(static_cast<tuple&&>(*this), static_cast<A&&>(Arg)), static_cast<tuple&&>(*this);
  }
  template<typename A> constexpr const tuple&& operator=(A&& Arg) const&& requires vassignable<const tuple&&, A> {
    return vassign(static_cast<const tuple&&>(*this), static_cast<A&&>(Arg)), static_cast<const tuple&&>(*this);
  }
};

template<typename T> struct tuple<T> {
  static constexpr size_t count{1};
  using first_type = T;
  first_type first;
  template<size_t I> requires(I < 1) constexpr auto get() & noexcept -> T& { return first; }
  template<size_t I> requires(I < 1) constexpr auto get() const& noexcept -> const T& { return first; }
  template<size_t I> requires(I < 1) constexpr auto get() && noexcept -> T&& { return static_cast<T&&>(first); }
  template<size_t I> requires(I < 1) constexpr auto get() const&& noexcept -> const T&& {
    return static_cast<T&&>(first);
  }
  template<typename A> constexpr tuple& operator=(A&& Arg) & requires vassignable<tuple&, A> {
    return vassign(*this, static_cast<A&&>(Arg)), *this;
  }
  template<typename A> constexpr const tuple& operator=(A&& Arg) const& requires vassignable<const tuple&, A> {
    return vassign(*this, static_cast<A&&>(Arg)), *this;
  }
  template<typename A> constexpr tuple&& operator=(A&& Arg) && requires vassignable<tuple&&, A> {
    return vassign(static_cast<tuple&&>(*this), static_cast<A&&>(Arg)), static_cast<tuple&&>(*this);
  }
  template<typename A> constexpr const tuple&& operator=(A&& Arg) const&& requires vassignable<const tuple&&, A> {
    return vassign(static_cast<const tuple&&>(*this), static_cast<A&&>(Arg)), static_cast<const tuple&&>(*this);
  }
};

template<> struct tuple<> {
  static constexpr size_t count = 0;
  template<typename... Ts> static constexpr auto asref(Ts&&... Args) noexcept {
    return tuple<Ts&&...>{static_cast<Ts&&>(Args)...};
  }
  template<specialization_of<typepack> Tp, typename Qualifier = none> using from_typepack =
    _::_tuple_from_typepack<Tp, Qualifier, remove_cvref<Qualifier>>::type;
};

template<typename... Ts> tuple(Ts...) -> tuple<Ts...>;

/// MARK: vapply_r

template<typename R> inline constexpr auto vapply_r = []<typename Fn, typename... Tps>(Fn&& fn, Tps&&... tps) -> R
  requires requires { requires((extent<R> == extent<Tps>) && ...); }
{
  constexpr auto foo = []<size_t I>(constant<I>, Fn & fn, Tps & ... tps) { return yw::invoke(fn, yw::get<I>(tps)...); };
  return [&foo]<size_t... Is>(sequence<Is...>, Fn & fn, Tps & ... tps) -> R {
    return construct<R>(foo(constant<Is>{}, fn, tps...)...);
  }(make_indices_for<R>{}, fn, tps...);
};

} // namespace yw

/// MARK: std

namespace std {

template<auto... Ts> struct tuple_size<yw::sequence<Ts...>> : integral_constant<size_t, sizeof...(Ts)> {};

template<size_t I, auto... Ts> struct tuple_element<I, yw::sequence<Ts...>>
  : type_identity<yw::select_type<I, decltype(Ts)...>> {};

template<typename... Ts> struct tuple_size<yw::typepack<Ts...>> : integral_constant<size_t, sizeof...(Ts)> {};

template<size_t I, typename... Ts> struct tuple_element<I, yw::typepack<Ts...>>
  : type_identity<yw::select_type<I, Ts...>> {};

template<typename... Ts> struct tuple_size<yw::tuple<Ts...>> : integral_constant<size_t, sizeof...(Ts)> {};

template<size_t I, typename... Ts> struct tuple_element<I, yw::tuple<Ts...>>
  : type_identity<yw::select_type<I, Ts...>> {};
} // namespace std
