/// \file core/list.hpp

#pragma once

#include "typepack.hpp"
#include "apply.hpp"


export namespace yw {


/// struct to represent a standard tuple-like object
template<typename... Ts> struct list
  : typepack<Ts...>::template fore<sizeof...(Ts) - 1> ::template expand<list> {

  /// base class
  using base = typepack<Ts...>::template fore<sizeof...(Ts) - 1> ::template expand<list>;

  /// number of elements
  static constexpr nat count = sizeof...(Ts);

  /// last element type
  using last_type = select_type<count - 1, Ts...>;

  /// last element
  select_type<count - 1, Ts...> last;

  /// returns the `I`-th element of `this`
  template<nat I> constexpr auto get() & noexcept
    -> select_type<I, Ts...>& {
    if constexpr (I == count - 1) return last;
    else return base::template get<I>();
  }

  /// returns the `I`-th element of `this` (const& version)
  template<nat I> constexpr auto get() const& noexcept
    -> add_const<select_type<I, Ts...>&> {
    if constexpr (I == count - 1) return last;
    else return base::template get<I>();
  }

  /// returns the `I`-th element of `this` (&& version)
  template<nat I> constexpr auto get() && noexcept
    -> select_type<I, Ts...>&& {
    if constexpr (I == count - 1) return fwd<last_type>(last);
    else return static_cast<base&&>(*this).template get<I>();
    // else return fwd<select_type<I, Ts...>>(base::template get<I>());
  }

  /// returns the `I`-th element of `this` (const&& version)
  template<nat I> constexpr auto get() const&& noexcept
    -> add_const<select_type<I, Ts...>&&> {
    if constexpr (I == count - 1) return fwd<add_const<last_type>>(last);
    else return static_cast<const base&&>(*this).template get<I>();
  }

  /// assigns corresponding elements of `Rt` to `this`
  template<typename Rt> constexpr void operator=(Rt&& R) &
    noexcept(nt_vassignable<list&, Rt>) requires vassignable<list&, Rt> {
    vassign(*this, fwd<Rt>(R));
  }

  /// assigns corresponding elements of `Rt` to `this` (const& version)
  template<typename Rt> constexpr void operator=(Rt&& R) const&
    noexcept(nt_vassignable<const list&, Rt>)
    requires vassignable<const list&, Rt> {
    vassign(*this, fwd<Rt>(R));
  }

  /// assigns corresponding elements of `Rt` to `this` (&& version)
  template<typename Rt> constexpr void operator=(Rt&& R) &&
    noexcept(nt_vassignable<list&&, Rt>) requires vassignable<list&&, Rt> {
    vassign(mv(*this), fwd<Rt>(R));
  }

  /// assigns corresponding elements of `Rt` to `this` (const&& version)
  template<typename Rt> constexpr void operator=(Rt&& R) const&&
    noexcept(nt_vassignable<const list&&, Rt>)
    requires vassignable<const list&&, Rt> {
    vassign(mv(*this), fwd<Rt>(R));
  }
};


/// struct to represent a standard tuple-like object
template<typename T1, typename T2, typename T3>
struct list<T1, T2, T3> : list<T1, T2> {

  /// number of elements
  static constexpr nat count = 3;

  /// third element type
  using third_type = T3;

  /// third element
  third_type third;

  /// returns the `I`-th element of `this`
  template<nat I> constexpr auto get() & noexcept
    -> select_type<I, T1, T2, T3>& {
    if constexpr (I == 2) return third;
    else return static_cast<list<T1, T2>&>(*this).template get<I>();
  }
  template<nat I> constexpr auto get() const& noexcept
    -> add_const<select_type<I, T1, T2, T3>&> {
    if constexpr (I == 2) return third;
    else return static_cast<const list<T1, T2>&>(*this).template get<I>();
  }
  template<nat I> constexpr auto get() && noexcept
    -> select_type<I, T1, T2, T3>&& {
    if constexpr (I == 2) return fwd<T3>(third);
    else return static_cast<list<T1, T2>&&>(*this).template get<I>();
  }
  template<nat I> constexpr auto get() const&& noexcept
    -> add_const<select_type<I, T1, T2, T3>&&> {
    if constexpr (I == 2) return fwd<add_const<T3>>(third);
    else return static_cast<const list<T1, T2>&&>(*this).template get<I>();
  }
};

template<typename T1, typename T2> struct list<T1, T2> : public list<T1> {
  using base = list<T1>;
  static constexpr nat count = 2;
  using second_type = T2;
  second_type second;

  template<nat I> constexpr auto get() & noexcept -> select_type<I, T1, T2>& {
    if constexpr (I == 1) return second;
    else return base::first;
  }
  template<nat I> constexpr auto get() const& noexcept -> add_const<select_type<I, T1, T2>&> {
    if constexpr (I == 1) return second;
    else return base::first;
  }
  template<nat I> constexpr auto get() && noexcept -> select_type<I, T1, T2>&& {
    if constexpr (I == 1) return fwd<T2>(second);
    else return fwd<T1>(base::first);
  }
  template<nat I> constexpr auto get() const&& noexcept -> add_const<select_type<I, T1, T2>&&> {
    if constexpr (I == 1) return fwd<add_const<T2>>(second);
    else return fwd<add_const<T1>>(base::first);
  }
};

template<typename T> struct list<T> {
  static constexpr nat count = 1;
  using first_type = T;
  first_type first;
  template<nat I> requires(I == 0) constexpr T& get() & noexcept { return first; }
  template<nat I> requires(I == 0) constexpr add_const<T&> get() const& noexcept { return first; }
  template<nat I> requires(I == 0) constexpr T&& get() && noexcept { return fwd<T>(first); }
  template<nat I> requires(I == 0) constexpr add_const<T&&> get() const&& noexcept { return fwd<add_const<T>>(first); }
};

template<> struct list<> {
private:
  template<typename T, typename U, typename V> struct _from_typepack {};
  template<typename... Ts, typename U, template<typename...> typename Tm, typename... Vs>
  struct _from_typepack<typepack<Ts...>, U, Tm<Vs...>> : _::_type<list<copy_cvref<Tm<Ts>, U>...>> {};
  template<typename... Ts, typename U, template<typename, auto...> typename Tm, typename V, auto... Vs>
  struct _from_typepack<typepack<Ts...>, U, Tm<V, Vs...>> : _::_type<list<copy_cvref<Tm<Ts, Vs...>, U>...>> {};
  template<typename... Ts, typename U, typename V> struct _from_typepack<typepack<Ts...>, U, V> : _::_type<list<copy_cvref<Ts, U>...>> {};
public:
  static constexpr nat count = 0;
  template<typename... Ts> static constexpr auto asref(Ts&&... Args) noexcept { return list<Ts&&...>{fwd<Ts>(Args)...}; }
  template<specialization_of<typepack> Tp, typename Qualifier = none> using from_typepack = _from_typepack<Tp, Qualifier, remove_cvref<Qualifier>>::type;
};

template<typename... Ts> list(Ts...) -> list<Ts...>;

namespace _ {

template<nat I, typename F, typename... Ts>
constexpr decltype(auto) _vapply_i(F&& f, Ts&&... ts)
  noexcept(nt_invocable<F, element_t<Ts, I>...>)
  requires invocable<F, element_t<Ts, I>...>
{ return invoke(fwd<F>(f), get<I>(fwd<Ts>(ts))...); }

template<nat... Is, typename F, typename... Ts>
constexpr decltype(auto) _vapply_is(sequence<Is...>, F&& f, Ts&&... ts)
  ywlib_wrapper(list{_vapply_i<Is>(fwd<F>(f), fwd<Ts>(ts)...)...});

template<typename F, typename T, typename... Ts>
requires same_extent<T, Ts...>
constexpr decltype(auto) _vapply(F&& f, T&& t, Ts&&... ts)
  ywlib_wrapper(_vapply_is(make_indices_for<T>(), fwd<F>(f), fwd<T>(t), fwd<Ts>(ts)...));

template<typename F, typename T, typename... Ts>
requires (!same_extent<T, Ts...> && tuple<T> && (tuple<Ts> && ...))
void _vapply(F&& f, T&& t, Ts&&... ts) = delete;

template<typename F, typename... Ts>
requires (!(tuple<Ts> || ...))
void _vapply(F&& f, Ts&&... ts) = delete;

template<nat I, nat N, nat... Is, nat... Js, typename F, typename... Ts>
constexpr decltype(auto) _vapply_b(sequence<Is...>, sequence<Js...>, F&& f, Ts&&... ts)
  ywlib_wrapper(_vapply(fwd<F>(f), select<Is>(fwd<Ts>(ts)...)...,
    projector(select<I>(fwd<Ts>(ts)...), make_sequence<0, N>{}),
    select<Js>(fwd<Ts>(ts)...)...));

template<nat I, nat J, typename F, typename... Ts>
constexpr decltype(auto) _vapply_a(F&& f, Ts&&... ts)
  ywlib_wrapper(_vapply_b<I, extent<select_type<J, Ts...>>>(
    make_sequence<0, I>{}, make_sequence<I + 1, sizeof...(Ts)>{}, fwd<F>(f), fwd<Ts>(ts)...));

template<typename F, typename... Ts>
requires (!(tuple<Ts> && ...) && (tuple<Ts> || ...))
constexpr decltype(auto) _vapply(F&& f, Ts&&... ts)
  ywlib_wrapper(_vapply_a<inspects<!tuple<Ts>...>,
    inspects<tuple<Ts>...>>(fwd<F>(f), fwd<Ts>(ts)...));
}

template<typename T, typename... Ts> concept vapplyable =
  requires(T&& t, Ts&&... ts) { { _::_vapply(fwd<T>(t), fwd<Ts>(ts)...) }; };

template<typename T, typename... Ts> concept nt_vapplyable = requires(T&& t, Ts&&... ts) {
  requires vapplyable<T, Ts...>;
  { _::_vapply(fwd<T>(t), fwd<Ts>(ts)...) } noexcept; };

template<typename T, typename... Ts> using vapply_result =
  decltype(_::_vapply(declval<T>(), declval<Ts>()...));

inline constexpr auto vapply = []<typename F, typename... Ts>(F&& f, Ts&&... ts)
  noexcept(nt_vapplyable<F, Ts...>) requires vapplyable<F, Ts...>
{ return _::_vapply(fwd<F>(f), fwd<Ts>(ts)...); };


} ////////////////////////////////////////////////////////////////////////////// namespace yw

namespace std {

template<typename... Ts> struct tuple_size<yw::list<Ts...>> : integral_constant<size_t, sizeof...(Ts)> {};
template<size_t I, typename... Ts> struct tuple_element<I, yw::list<Ts...>> { using type = yw::select_type<I, Ts...>; };

} ////////////////////////////////////////////////////////////////////////////// namespace std
