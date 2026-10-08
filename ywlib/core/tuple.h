#pragma once
#include <core/core.h>

namespace yw {

template<auto... Vs> struct sequence;

namespace internal {
template<typename T, typename U> struct _to_sequence : _type<void> {};
template<template<auto...> typename Tm, auto... Vs, typename U> struct _to_sequence<Tm<Vs...>, U>
  : _type<sequence<static_cast<U>(Vs)...>> {};
template<template<typename T, T...> typename Tm, typename T, T... Vs, typename U> struct _to_sequence<Tm<T, Vs...>, U>
  : _type<sequence<static_cast<U>(Vs)...>> {};
template<template<typename T, T...> typename Tm, typename T, T... Vs> struct _to_sequence<Tm<T, Vs...>, none>
  : _type<sequence<Vs...>> {};
template<template<auto...> typename Tm, auto... Vs> struct _to_sequence<Tm<Vs...>, none> : _type<sequence<Vs...>> {};
} // namespace internal

template<typename T, typename U = none> using to_sequence = internal::_to_sequence<T, U>::type;
template<typename T, typename U = none> concept is_sequence = !is_void<to_sequence<T, U>>;

namespace internal {
template<typename Sq, size_t N> inline constexpr bool _indices_for = false;
template<size_t... Is, size_t N> inline constexpr bool _indices_for<sequence<Is...>, N> = ((Is < N) && ...);
} // namespace internal

template<typename Sq, typename Tp> concept indices_for = internal::_indices_for<to_sequence<Sq, size_t>, extent<Tp>>;

namespace internal {
template<size_t I, size_t N, auto P, auto... Vs> struct _make_sequence
  : select_type<gt(I, N), _type<sequence<>>, _make_sequence<I + 1, N, P, Vs..., P(I)>> {};
template<size_t N, auto P, auto... Vs> struct _make_sequence<N, N, P, Vs...> : _type<sequence<Vs...>> {};
} // namespace internal

template<size_t Begin, size_t End, auto Proj = pass{}> requires invocable<decltype(Proj), size_t>
using make_sequence = internal::_make_sequence<Begin, End, Proj>::type;
template<typename T> using make_indices_for = make_sequence<0, extent<T>>;

namespace internal {
template<typename S, size_t... Is> struct _extracting_indices : _type<void> {};
template<bool... Bs> struct _extracting_indices<sequence<Bs...>>
  : _extracting_indices<sequence<Bs...>, 0, sizeof...(Bs)> {};
template<bool... Bs, size_t I, size_t N, size_t... Is> struct _extracting_indices<sequence<Bs...>, I, N, Is...>
  : select_type<
      select(I, Bs...), _extracting_indices<sequence<Bs...>, I + 1, N, Is..., I>,
      _extracting_indices<sequence<Bs...>, I + 1, N, Is...>> {};
template<bool... Bs, size_t N, size_t... Is> struct _extracting_indices<sequence<Bs...>, N, N, Is...>
  : _type<sequence<Is...>> {};
} // namespace internal

template<is_sequence<bool> S> using extracting_indices = internal::_extracting_indices<to_sequence<S, bool>>::type;

namespace internal {
template<typename S, typename T> struct _sequence_extract : _type<void> {};
template<auto... Vs, size_t... Is> struct _sequence_extract<sequence<Vs...>, sequence<Is...>>
  : _type<sequence<select(Is, Vs...)...>> {};
template<typename S, typename T> struct _sequence_append : _type<void> {};
template<auto... Vs, auto... Ws> struct _sequence_append<sequence<Vs...>, sequence<Ws...>>
  : _type<sequence<Vs..., Ws...>> {};
} // namespace internal

template<auto... Vs> struct sequence {
  static constexpr size_t count{sizeof...(Vs)};
  template<size_t I> requires(I < sizeof...(Vs)) static constexpr auto at = select(I, Vs...);
  template<size_t I> requires(I < sizeof...(Vs)) using type_at = select_type<I, decltype(Vs)...>;
  template<indices_for<sequence> Ix> using extract = internal::_sequence_extract<sequence, Ix>;
  template<size_t N> requires(N <= sizeof...(Vs)) using fore = extract<make_sequence<0, N>>;
  template<size_t N> requires(N <= sizeof...(Vs)) using back = extract<make_sequence<sizeof...(Vs) - N, sizeof...(Vs)>>;
  template<is_sequence Sq> using append = internal::_sequence_append<sequence, to_sequence<Sq>>::type;
  template<size_t I> requires(I < sizeof...(Vs)) using remove = fore<I>::template append<back<sizeof...(Vs) - I - 1>>;
  template<size_t I, is_sequence Sq> requires(I <= sizeof...(Vs))
  using insert = typename fore<I>::template append<Sq>::template append<back<sizeof...(Vs) - I>>;
  template<template<auto...> typename Tm> using expand = Tm<Vs...>;
  using reverse = extract<make_sequence<0, sizeof...(Vs), [](size_t I) { return sizeof...(Vs) - I - 1; }>>;
  template<size_t I> requires(I < sizeof...(Vs)) constexpr const auto&& get() const noexcept { return move(at<I>); }
};

template<typename... Ts> struct typepack;

namespace internal {
template<typename T, typename S> struct _to_typepack : _type<void> {};
template<typename T, size_t... Is> struct _to_typepack<T, sequence<Is...>> : _type<typepack<element_t<T, Is>...>> {};
} // namespace internal

template<typename T> using to_typepack = internal::_to_typepack<T, make_indices_for<T>>::type;

namespace internal {
template<typename T, typename Is> struct _typepack_extract : _type<void> {};
template<typename... Ts, size_t... Is> struct _typepack_extract<typepack<Ts...>, sequence<Is...>>
  : _type<typepack<select_type<Is, Ts...>...>> {};
template<typename T, typename U> struct _typepack_append : _type<void> {};
template<typename... Ts, typename... Us> struct _typepack_append<typepack<Ts...>, typepack<Us...>>
  : _type<typepack<Ts..., Us...>> {};
} // namespace internal

template<typename... Ts> struct typepack {
  static constexpr size_t count = sizeof...(Ts);
  template<size_t I> requires(I < sizeof...(Ts)) using at = select_type<I, Ts...>;
  template<indices_for<typepack> Ix> using extract =
    internal::_typepack_extract<typepack, to_sequence<Ix, size_t>>::type;
  template<size_t N> requires(N <= sizeof...(Ts)) using fore = extract<make_sequence<0, N>>;
  template<size_t N> requires(N <= sizeof...(Ts)) using back = extract<make_sequence<sizeof...(Ts) - N, sizeof...(Ts)>>;
  template<specialization_of<yw::typepack> T> using append = internal::_typepack_append<typepack, T>;
  template<size_t I> requires(I < sizeof...(Ts)) using remove = fore<I>::template append<back<sizeof...(Ts) - I - 1>>;
  template<size_t I, specialization_of<yw::typepack> T> requires(I <= sizeof...(Ts))
  using insert = typename fore<I>::template append<T>::template append<back<sizeof...(Ts) - I>>;
  using reverse = extract<make_sequence<0, sizeof...(Ts), [](size_t I) { return sizeof...(Ts) - I - 1; }>>;
  template<template<typename...> typename Tm> using expand = Tm<Ts...>;
  template<size_t I> requires(I < sizeof...(Ts)) constexpr const at<I> get() const noexcept;
};

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

template<typename F, typename Tp> concept applyable = requires { apply(declval<F>(), declval<Tp>()); };
template<typename F, typename Tp> concept nt_applyable = noexcept(apply(declval<F>(), declval<Tp>()));
template<typename F, typename Tp> using apply_result = decltype(apply(declval<F>(), declval<Tp>()));

template<typename T, typename Tp> concept buildable = applyable<decltype(construct<T>), Tp>;
template<typename T, typename Tp> concept nt_buildable = nt_applyable<decltype(construct<T>), Tp>;
template<typename T> inline constexpr auto build = []<typename Tp>(Tp&& Tuple) noexcept(nt_buildable<T, Tp>) -> T
  requires buildable<T, Tp> { return apply(construct<T>, static_cast<Tp&&>(Tuple)); };

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
    noexcept(internal::operator()<0, extent<T>>(f, declval<T&&>(), declval<Ts&&>()...)))
    requires requires { internal::operator()<0, extent<T>>(f, declval<T&&>(), declval<Ts&&>()...); } {
    internal::operator()<0, extent<T>>(f, static_cast<T&&>(t), static_cast<Ts&&>(ts)...);
  }
} vapply;

template<typename F, typename... Ts> concept vapplyable = requires { vapply(declval<F>(), declval<Ts>()...); };
template<typename F, typename... Ts> concept nt_vapplyable = noexcept(vapply(declval<F>(), declval<Ts>()...));

template<typename T, typename U> concept vassignable = vapplyable<decltype(assign), T, U>;
template<typename T, typename U> concept nt_vassignable = nt_vapplyable<decltype(assign), T, U>;
inline constexpr auto vassign = []<typename T, typename U>(T&& t, U&& u) noexcept(nt_vassignable<T, U>) -> void
  requires vassignable<T, U> { vapply(assign, static_cast<T&&>(t), static_cast<U&&>(u)); };

template<typename R> inline constexpr auto vapply_r = []<typename Fn, typename... Tps>(Fn&& fn, Tps&&... tps) -> R
  requires((extent<R> == extent<Tps>) && ...) {
    constexpr auto foo = []<size_t I>(constant<I>, Fn& fn, Tps&... tps) { return yw::invoke(fn, yw::get<I>(tps)...); };
    return [&foo]<size_t... Is>(sequence<Is...>, Fn& fn, Tps&... tps) -> R {
      return construct<R>(foo(constant<Is>{}, fn, tps...)...);
    }(make_indices_for<R>{}, fn, tps...);
  };

///--------------------------------------------------------------------------///
/// MARK: tuple

inline constexpr struct arg_separator {
} arg_separator;

namespace internal {
template<typename... Ts> struct _tuple_args {};
template<typename T> struct _tuple_args<T> {
  T&& ref;
  constexpr _tuple_args(T&& r) : ref(static_cast<T&&>(r)) {}
  template<size_t I> constexpr decltype(auto) get() noexcept { return static_cast<T&&>(ref); }
};
template<typename T, typename... Ts> requires(sizeof...(Ts) > 0) struct _tuple_args<T, Ts...> {
  T&& ref;
  _tuple_args<Ts...> rest;
  constexpr _tuple_args(T&& r, Ts&&... rs) : ref(static_cast<T&&>(r)), rest(static_cast<Ts&&>(rs)...) {}
  template<size_t I> constexpr decltype(auto) get() noexcept {
    if constexpr (I == 0) return static_cast<T&&>(ref);
    else return rest.template get<I - 1>();
  }
};
template<typename... Ts> _tuple_args(Ts&&...) -> _tuple_args<Ts...>;
template<typename... Ts> constexpr _tuple_args<Ts&&...> _make_tuple_args(Ts&&... ts) {
  return _tuple_args<Ts&&...>(static_cast<Ts&&>(ts)...);
}
template<typename... Ts> struct _tuple_base {};
template<typename T> struct _tuple_base<T> {
  using first_type = T;
  first_type first;
  constexpr _tuple_base() requires constructible<first_type> : first() {}
  template<typename Args, size_t... Is> constexpr _tuple_base(Args&& args, typepack<sequence<Is...>>) //
    noexcept(noexcept(T(args.template get<Is>()...))) requires requires { T(args.template get<Is>()...); }
    : first(args.template get<Is>()...) {}
  template<size_t I, typename Self> constexpr decltype(auto) get(this Self&& self) noexcept {
    return static_cast<copy_cvref_weak<Self&&, T>>(self.first);
  }
};
template<typename T, typename U> struct _tuple_base<T, U> : _tuple_base<T> {
  using base = _tuple_base<T>;
  using second_type = U;
  second_type second;
  constexpr _tuple_base() requires constructible<base> && constructible<second_type> : second() {}
  template<typename Args, size_t... Is, typename Rest>
  constexpr _tuple_base(Args&& args, typepack<sequence<Is...>, Rest>)                                 //
    noexcept(noexcept(U(args.template get<Is>()...)) && noexcept(base(move(args), typepack<Rest>()))) //
    requires requires { U(args.template get<Is>()...), base(move(args), typepack<Rest>()); }
    : _tuple_base<T>(move(args), typepack<Rest>()), second(args.template get<Is>()...) {}
  template<size_t I, typename Self> constexpr decltype(auto) get(this Self&& self) noexcept {
    if constexpr (I == 0) return static_cast<copy_cvref_weak<Self&&, T>>(self.first);
    else return static_cast<copy_cvref_weak<Self&&, U>>(self.second);
  }
};
template<typename T, typename U, typename V> struct _tuple_base<T, U, V> : _tuple_base<T, U> {
  using base = _tuple_base<T, U>;
  using third_type = V;
  third_type third;
  constexpr _tuple_base() requires constructible<base> && constructible<third_type> : third() {}
  template<typename Args, size_t... Is, typename... Rests>
  constexpr _tuple_base(Args&& args, typepack<sequence<Is...>, Rests...>)                                 //
    noexcept(noexcept(V(args.template get<Is>()...)) && noexcept(base(move(args), typepack<Rests...>()))) //
    requires requires { V(args.template get<Is>()...), base(move(args), typepack<Rests...>()); }
    : _tuple_base<T, U>(move(args), typepack<Rests...>()), third(args.template get<Is>()...) {}
  template<size_t I, typename Self> constexpr decltype(auto) get(this Self&& self) noexcept {
    if constexpr (I == 0) return static_cast<copy_cvref_weak<Self&&, T>>(self.first);
    else if constexpr (I == 1) return static_cast<copy_cvref_weak<Self&&, U>>(self.second);
    else return static_cast<copy_cvref_weak<Self&&, V>>(self.third);
  }
};
template<typename... Ts> requires(sizeof...(Ts) > 3)
struct _tuple_base<Ts...> : typepack<Ts...>::template fore<sizeof...(Ts) - 1>::template expand<_tuple_base> {
  using base = typepack<Ts...>::template fore<sizeof...(Ts) - 1>::template expand<_tuple_base>;
  using last_type = Ts...[sizeof...(Ts) - 1];
  last_type last;
  constexpr _tuple_base() requires constructible<base> && constructible<last_type> : last() {}
  template<typename Args, size_t... Is, typename... Rests>
  constexpr _tuple_base(Args&& args, typepack<sequence<Is...>, Rests...>)                                         //
    noexcept(noexcept(last_type(args.template get<Is>()...)) && noexcept(base(move(args), typepack<Rests...>()))) //
    requires requires { last_type(args.template get<Is>()...), base(move(args), typepack<Rests...>()); }
    : base(move(args), typepack<Rests...>()), last(args.template get<Is>()...) {}
  template<size_t I, typename Self> constexpr decltype(auto) get(this Self&& self) noexcept {
    if constexpr (I == sizeof...(Ts) - 1) return static_cast<copy_cvref_weak<Self&&, last_type>>(self.last);
    else return static_cast<copy_cvref_weak<Self&&, base>>(self).template get<I>();
  }
};
template<typename... As> consteval auto _make_tuple_ids(typepack<As...>) noexcept {
  constexpr auto arg_count = sizeof...(As);
  constexpr auto separator_index = inspect(same_as<remove_cvref<As>, decltype(arg_separator)>...);
  using type = make_sequence<0, separator_index>;
  if constexpr (separator_index != arg_count) {
    using rest = decltype(_make_tuple_ids(typepack<As...>::template back<arg_count - separator_index - 1>()));
    return rest::template append<type>();
  } else return typepack<type>();
}
template<size_t... Is> consteval auto _make_tuple_ids(sequence<Is...>) noexcept {
  return typename typepack<sequence<Is>...>::reverse();
}
} // namespace internal

template<typename... Ts> struct tuple : internal::_tuple_base<Ts...> {
  static constexpr size_t count = sizeof...(Ts);
  using base = internal::_tuple_base<Ts...>;
  constexpr tuple() requires constructible<internal::_tuple_base<Ts...>> = default;
  template<typename... As> requires(sizeof...(As) > sizeof...(Ts)) constexpr tuple(As&&... as) noexcept(
    noexcept(base(internal::_make_tuple_args(static_cast<As&&>(as)...), internal::_make_tuple_ids(typepack<As...>()))))
    requires constructible<base, internal::_tuple_args<As&&...>, decltype(internal::_make_tuple_ids(typepack<As...>()))>
    : base(internal::_make_tuple_args(static_cast<As&&>(as)...), internal::_make_tuple_ids(typepack<As...>())) {}
  template<typename... As> constexpr tuple(As&&... as) noexcept((nt_constructible<Ts, As> && ...))
    requires((constructible<Ts, As> && ...) && sizeof...(Ts) == sizeof...(As))
    : base(
        internal::_make_tuple_args(static_cast<As&&>(as)...),
        decltype(internal::_make_tuple_ids(make_sequence<0, sizeof...(As)>()))()) {}
  template<tuple_like<count> Tp> constexpr tuple(Tp&& tp) : tuple(make_indices_for<Tp>(), static_cast<Tp&&>(tp)) {}
  template<size_t... Is, tuple_like<count> Tp> constexpr tuple(sequence<Is...>, Tp&& tp)
    : base(
        internal::_make_tuple_args(yw::get<Is>(static_cast<Tp&&>(tp))...),
        internal::_make_tuple_ids(sequence<Is...>())) {}
  template<tuple_like<count> Tp> constexpr tuple& operator=(Tp&& tp) noexcept(nt_vassignable<tuple&, Tp&&>)
    requires vassignable<tuple&, Tp&&> {
    vassign(*this, static_cast<Tp&&>(tp));
    return *this;
  }
  template<size_t I, typename Self> constexpr decltype(auto) get(this Self&& self) noexcept {
    return static_cast<copy_cvref_weak<Self&&, select_type<I, Ts...>>>(static_cast<base&&>(self).template get<I>());
  }
};

template<typename... Ts> tuple(Ts&&...) -> tuple<remove_cvref<Ts>...>;

///--------------------------------------------------------------------------///
/// MARK: projector

template<is_reference Rf, typename Pj, variation_of<sequence<>> Sq> struct projector;

template<is_reference Rf, typename Pj, size_t... Is> //
requires tuple_like<Rf> && indices_for<sequence<Is...>, Rf> struct projector<Rf, Pj, sequence<Is...>> {
  Rf ref;
  Pj proj;
  constexpr projector(Rf r, Pj p) noexcept(nt_constructible<Pj, Pj>) requires constructible<Pj, Pj>
    : ref(static_cast<Rf>(r)), proj(static_cast<Pj>(p)) {}
  template<indices_for<Rf> S> constexpr projector(Rf r, Pj p, S) noexcept(nt_constructible<Pj, Pj>)
    requires constructible<Pj, Pj>
    : ref(static_cast<Rf>(r)), proj(static_cast<Pj>(p)) {}
  template<indices_for<Rf> S> requires constructible<Pj>
  constexpr projector(Rf r, S) noexcept(nt_constructible<Pj>) : ref(static_cast<Rf>(r)), proj() {}

  template<size_t I, typename Self> requires(lt(I, sizeof...(Is)))
  constexpr decltype(auto) get(this Self&& self) noexcept(nt_invocable<Pj&, element_t<Rf&, Is...[I]>>) {
    return yw::invoke(self.proj, yw::get<Is...[I]>(static_cast<copy_ref_weak<Self&&, Rf>>(self.ref)));
  }
};

template<tuple_like R, vapplyable<R> P, indices_for<R> S> //
projector(R&&, P&&, S) -> projector<R&&, P&&, to_sequence<S, size_t>>;
template<tuple_like R, vapplyable<R> P> requires(!is_sequence<P>)
projector(R&&, P&&) -> projector<R&&, P&&, make_indices_for<R>>;
template<tuple_like R, indices_for<R> S> projector(R&&, S) -> projector<R&&, pass, to_sequence<S, size_t>>;

template<is_reference Rf, typename Pj, size_t... Is> requires(!tuple_like<Rf>)
struct projector<Rf, Pj, sequence<Is...>> {
  Rf ref;
  Pj proj;
  constexpr projector(Rf r, Pj p) noexcept(nt_constructible<Pj, Pj>) requires constructible<Pj, Pj>
    : ref(static_cast<Rf>(r)), proj(static_cast<Pj>(p)) {}
  template<is_sequence<size_t> S> constexpr projector(Rf r, Pj p, S) noexcept(nt_constructible<Pj, Pj>)
    requires constructible<Pj, Pj>
    : ref(static_cast<Rf>(r)), proj(static_cast<Pj>(p)) {}
  template<is_sequence<size_t> S> constexpr projector(Rf r, S) noexcept(nt_constructible<Pj>) requires constructible<Pj>
    : ref(static_cast<Rf>(r)), proj() {}

  template<size_t I, typename Self> requires(lt(I, sizeof...(Is)))
  constexpr decltype(auto) get(this Self&& self) noexcept(nt_invocable<Pj&, copy_ref_weak<Self&&, Rf>>) {
    return yw::invoke(self.proj, static_cast<copy_ref_weak<Self&&, Rf>>(self.ref));
  }
};

template<typename R, vapplyable<R> P, is_sequence<size_t> S> requires(!tuple_like<R>)
projector(R&&, P&&, S) -> projector<R&&, P&&, to_sequence<S, size_t>>;
template<typename R, vapplyable<R> P> requires(!tuple_like<R> && !is_sequence<P>)
projector(R&&, P&&) -> projector<R&&, P&&, sequence<static_cast<size_t>(0)>>;
template<typename R, is_sequence<size_t> S> requires(!tuple_like<R>)
projector(R&&, S) -> projector<R&&, pass, to_sequence<S, size_t>>;
} // namespace yw

namespace std {

template<auto... Ts> struct tuple_size<yw::sequence<Ts...>> : integral_constant<size_t, sizeof...(Ts)> {};
template<typename... Ts> struct tuple_size<yw::typepack<Ts...>> : integral_constant<size_t, sizeof...(Ts)> {};
template<typename... Ts> struct tuple_size<yw::tuple<Ts...>> : integral_constant<size_t, sizeof...(Ts)> {};
template<typename R, typename P, size_t... Is> struct tuple_size<yw::projector<R, P, yw::sequence<Is...>>>
  : integral_constant<size_t, sizeof...(Is)> {};

template<size_t I, auto... Ts> struct tuple_element<I, yw::sequence<Ts...>>
  : type_identity<yw::select_type<I, decltype(Ts)...>> {};
template<size_t I, typename... Ts> struct tuple_element<I, yw::typepack<Ts...>>
  : type_identity<yw::select_type<I, Ts...>> {};
template<size_t I, typename... Ts> struct tuple_element<I, yw::tuple<Ts...>>
  : type_identity<yw::select_type<I, Ts...>> {};
template<size_t I, yw::tuple_like R, typename P, size_t... Is>
struct tuple_element<I, yw::projector<R, P, yw::sequence<Is...>>>
  : type_identity<yw::invoke_result<P&, yw::element_t<R, Is... [I]>>> {};
template<size_t I, typename R, typename P, size_t... Is> requires(!yw::tuple_like<R>)
struct tuple_element<I, yw::projector<R, P, yw::sequence<Is...>>> : type_identity<yw::invoke_result<P&, R>> {};
} // namespace std
