#pragma once
#include "yw.hpp"
#ifdef VSCODE
#include <type_traits>
#include <concepts>
#else
import <type_traits>;
import <concepts>;
#endif

namespace yw {

  namespace _zyx {
    template<typename T> [[noreturn]] void _implicitly_default_constructible_f(const T&) { static_assert(false); }
    template<typename T>struct _is_memptr { using t = T; static constexpr bool v = false; };
    template<typename T, typename U>struct _is_memptr<T U::*> { using t = U; static constexpr bool v = true; };
    template<typename T> [[noreturn]] constant_agent<bool, true>_is_class_union_f(int T::*)noexcept { static_assert(false); };
    template<typename> [[noreturn]] constant_agent<bool, false>_is_class_union_f(...)noexcept { static_assert(false); };
    template<typename T, template<typename...> typename U>inline constexpr bool _specialized_of = false;
    template<template<typename...> typename U, typename...Args>inline constexpr bool _specialized_of<U<Args...>, U> = true;
  }
  namespace type {
    template<typename T, typename To> concept expl_convertible_to = requires(type::add_rvref<T>(&f)(void)) { static_cast<To>(f()); };
    template<typename T, typename To> concept nt_expl_convertible_to = expl_convertible_to<T, To> && requires(type::add_rvref<T>(&f)(void)noexcept) { { static_cast<To>(f()) }noexcept; };
    template<typename T, typename To> concept impl_convertible_to = std::is_convertible_v<T, To>;
    template<typename T, typename To> concept nt_impl_convertible_to = impl_convertible_to<T, To> && std::is_nothrow_convertible_v<T, To>;
    template<typename T, typename To> concept convertible_to = std::convertible_to<T, To> && expl_convertible_to<T, To> && impl_convertible_to<T, To>;
    template<typename T, typename To> concept nt_convertible_to = convertible_to<T, To> && nt_expl_convertible_to<T, To> && nt_impl_convertible_to<T, To>;

    template<typename...Ts> using common_type = std::common_type_t<Ts...>;
    template<typename...Ts> using common_reference = std::common_reference_t<Ts...>;
    template<typename T, typename U> concept has_common_type_with = std::common_with<T, U>;
    template<typename T, typename U> concept has_common_reference_with = std::common_reference_with<T, U>;
    
    template<typename T> concept destructible = std::destructible<T>;
    template<typename T> concept nt_destructible = destructible<T>;
    template<typename T> concept has_virtual_destructor = std::has_virtual_destructor_v<T>;

    template<typename T, typename...Args> concept constructible_from = std::constructible_from<T, Args...>;
    template<typename T, typename...Args> concept nt_constructible_from = constructible_from<T, Args...> && std::is_nothrow_constructible_v<T, Args...>;
    template<typename T, typename...Args> concept triv_constructible_from = constructible_from<T, Args...> && std::is_trivially_constructible_v<T, Args...>;
    template<typename T> concept default_constructible = std::default_initializable<T>;
    template<typename T> concept nt_default_constructible = default_constructible<T> && requires { { T{} }noexcept; { ::new (static_cast<void*>(nullptr)) T }noexcept; };
    template<typename T> concept triv_default_constructible = default_constructible<T> && triv_constructible_from<T>;
    template<typename T> concept impl_default_constructible = default_constructible<T> && requires { _zyx::_implicitly_default_constructible_f<T>({}); };
    template<typename T> concept move_constructible = std::move_constructible<T>;
    template<typename T> concept nt_move_constructible = move_constructible<T> && nt_constructible_from<T, T> && nt_convertible_to<T, T>;
    template<typename T> concept triv_move_constructible = move_constructible<T> && triv_constructible_from<T, T>;
    template<typename T> concept copy_constructible = std::copy_constructible<T>;
    template<typename T> concept nt_copy_constructible = copy_constructible<T> && nt_constructible_from<T, T&> && nt_constructible_from<T, const T&> && nt_constructible_from<T, const T> && nt_convertible_to<T&, T> && nt_convertible_to<const T&, T> && nt_convertible_to<const T, T>;
    template<typename T> concept triv_copy_constructible = copy_constructible<T> && triv_constructible_from<T, T&> && triv_constructible_from<T, const T&> && triv_constructible_from<T, const T> && nt_convertible_to<T&, T> && nt_convertible_to<const T&, T> && nt_convertible_to<const T, T>;

    template<typename T, typename Arg> concept assignable_from = std::assignable_from<T, Arg>;
    template<typename T, typename Arg> concept nt_assignable_from = assignable_from<T, Arg> && requires(T t, Arg&& arg) { { t = static_cast<Arg&&>(arg) }noexcept -> same_as<T>; };
    template<typename T, typename Arg> concept triv_assignable_from = assignable_from<T, Arg> && std::is_trivially_assignable_v<T, Arg>;
    template<typename T> concept move_assignable = assignable_from<T&, T> && convertible_to<T, T&>;
    template<typename T> concept nt_move_assignable =nt_assignable_from<T&, T> && nt_convertible_to<T, T&>;
    template<typename T> concept triv_move_assignable = move_assignable<T> && triv_assignable_from<T&, T>;
    template<typename T> concept copy_assignable = assignable_from<T&, T&> && assignable_from<T&, const T&> && assignable_from<T&, const T> && convertible_to<T&, T&> && convertible_to<const T&, T&> && convertible_to<const T, T&>;
    template<typename T> concept nt_copy_assignable = nt_assignable_from<T&, T&> && nt_assignable_from<T&, const T&> && nt_assignable_from<T&, const T> && nt_convertible_to<T&, T&> && nt_convertible_to<const T&, T&> && nt_convertible_to<const T, T&>;
    template<typename T> concept triv_copy_assignable = copy_assignable<T> && triv_assignable_from<T&, T&> && triv_assignable_from<T&, const T&> && triv_assignable_from<T&, const T>;
    
    template<typename T> concept triv_copyable = std::is_trivially_copyable_v<T>;
    template<typename T> concept trivial = triv_copyable<T> && triv_default_constructible<T>;
    

    template<typename T> concept is_memptr = _zyx::_is_memptr<T>::v;
    template<typename T> concept is_memptr_function = is_memptr<T> && is_function<T>;
    template<typename T> concept is_memptr_object = is_memptr<T> && !is_function<T>;
    template<typename T> concept is_enum = expl_convertible_to<T, bool> && expl_convertible_to<bool, T> && !constructible_from<T, bool> && !assignable_from<rem_ref<T>&, bool>;
    template<typename T> concept is_scoped_enum = is_enum<T> && !impl_convertible_to<bool, T>;
    template<typename T> concept is_unscoped_enum = is_enum<T> && impl_convertible_to<bool, T>;
    template<typename T> concept is_class_union = decltype(_is_class_union_f<T>(nullptr))::value;
    template<typename T> concept is_union = is_class_union<T> && std::is_union_v<T>;
    template<typename T> concept is_class = is_class_union<T> && !is_union<T>;
    template<typename T> concept is_scalar = is_nullptr<T> || arithmetic<T> || is_memptr<T> || is_ptr<T> || is_enum<T>;
    template<typename T> concept is_object = is_scalar<T> || is_array<T> || is_class<T> || is_union<T>;

    template<typename T, typename Derived> concept is_base_of = is_class<T> && is_class<Derived> && std::is_base_of_v<T, Derived>;
    template<typename T, typename Base> concept derived_from = is_base_of<Base, T> && impl_convertible_to<const volatile T*, const volatile Base*>;
    template<typename T, template<typename...> typename Template> concept specialized_of = _zyx::_specialized_of<T, Template>;

    template<typename T> concept is_empty_class = is_class<T> && std::is_empty_v<T>;
    template<typename T> concept is_final_class = is_class<T> && std::is_final_v<T>;
    template<typename T> concept is_abstract_class = is_class<T> && std::is_abstract_v<T>;
    template<typename T> concept is_polymorphic_class = is_class<T> && std::is_polymorphic_v<T>;
    template<typename T> concept is_aggregate_class = is_class<T> && std::is_aggregate_v<T>;

    template<is_memptr T> using class_type = _zyx::_is_memptr<T>::t;
    template<is_enum T> using underlying_type = _zyx::underlying_type_t<T>;

    template<typename F, typename... Args> concept invocable = std::is_invocable_v<F, Args...>;
    template<typename F, typename... Args> concept nothrow_invocable = invocable<F, Args...> && std::is_nothrow_invocable_v<F, Args...>;
    template<typename R, typename F, typename... Args> concept invocable_r = invocable<F, Args...> && requires { { yw::invoke(yw::declval<F&&>(), yw::declval<Args&&>()...) } -> same_as<R>; };
    template<typename R, typename F, typename... Args> concept nothrow_invocable_r = nothrow_invocable<F, Args...> && requires { { yw::invoke(yw::declval<F&&>(), yw::declval<Args&&>()...) }noexcept -> same_as<R>; };

    template<typename T> concept swappable = requires(T& x, T& y) { yw::swap(x, y); };
    template<typename T> concept nothrow_swappable = swappable<T> && requires(T& x, T& y) { { yw::swap(x, y) }noexcept; };
    template<typename T, typename U> concept swappable_with = has_common_reference_with<T, U> && requires(T&& t, U&& u) { yw::swap(static_cast<T&&>(t), static_cast<T&&>(t)); yw::swap(static_cast<T&&>(t), static_cast<U&&>(u)); yw::swap(static_cast<U&&>(u), static_cast<T&&>(t)); yw::swap(static_cast<U&&>(u), static_cast<U&&>(u)); };
    template<typename T, typename U> concept nothrow_swappable_with = swappable_with<T, U> && requires(T&& t, U&& u) { { yw::swap(static_cast<T&&>(t), static_cast<T&&>(t)) }noexcept; { yw::swap(static_cast<T&&>(t), static_cast<U&&>(u)) }noexcept; { yw::swap(static_cast<U&&>(u), static_cast<T&&>(t)) }noexcept; { yw::swap(static_cast<U&&>(u), static_cast<U&&>(u)) }noexcept; };

    template<typename T> concept movable = is_object<T> && move_constructible<T> && move_assignable<T> && swappable<T>;
    template<typename T> concept copyable = copy_constructible<T> && movable<T> && copy_assignable<T>;

    template<typename T> concept boolean_testable_impl = convertible_to<T, bool>;
    template<typename T> concept boolean_testable = boolean_testable_impl<T> && requires(T && t) { { !static_cast<T&&>(t) } -> boolean_testable_impl; };
    template<typename T, typename U> concept half_equality_comparable_with = requires(const lvref_t<T> t, const lvref_t<U> u) { { t == u } -> boolean_testable; { t != u } -> boolean_testable; };
    template<typename T, typename U> concept weakly_equality_comparable_with = half_equality_comparable_with<T, U> && half_equality_comparable_with<U, T>;
    template<typename T> concept equality_comparable = half_equality_comparable_with<T, T>;
    template<typename T, typename U> concept equality_comparable_with = equality_comparable<T> && equality_comparable<U> && has_common_reference_with<const lvref_t<T>, const lvref_t<U>> && equality_comparable<common_reference<const lvref_t<T>, const lvref_t<U>>> && weakly_equality_comparable_with<T, U>;
    template<typename T, typename U> concept half_ordered = requires(const lvref_t<T> t, const lvref_t<U> u) { { t < u } -> boolean_testable; { t > u } -> boolean_testable; { t <= u } -> boolean_testable; { t >= u } -> boolean_testable; };
    template<typename T, typename U> concept partially_ordered_with = half_ordered<T, U> && half_ordered<U, T>;
    template<typename T> concept totally_ordered = equality_comparable<T> && half_ordered<T, T>;
    template<typename T, typename U> concept totally_ordered_with = totally_ordered<T> && totally_ordered<U> && equality_comparable_with<T, U> && totally_ordered<common_reference<const lvref_t<T>, const lvref_t<U>>> && partially_ordered_with<T, U>;

    template<typename T> concept semiregular = copyable<T> && default_constructible<T>;
    template<typename T> concept regular = semiregular<T> && equality_comparable<T>;
    template<typename F, typename... Args> concept regular_invocable = invocable<F, Args...>;
    template<typename F, typename... Args> concept predicate = regular_invocable<F, Args...> && boolean_testable<invoke_result<F, Args...>>;
    template<typename F, typename T, typename U> concept relation = predicate<F, T, T> && predicate<F, T, U> && predicate<F, U, T> && predicate<F, U, U>;
    template<typename F, typename T, typename U> concept equivalence_relation = relation<F, T, U>;
    template<typename F, typename T, typename U> concept strict_weak_order = relation<F, T, U>;
  }

}