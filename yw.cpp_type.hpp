#pragma once
#include "yw.hpp"
#ifdef VSCODE
#include <type_traits>
#include <concepts>
#else
import <type_traits>;
import <concepts>;
#endif

namespace yw { namespace cpp {

template<typename T> using add_const = std::add_const_t<T>;
template<typename T> using add_volatile = std::add_volatile_t<T>;
template<typename T> using add_cv = std::add_cv_t<T>;
template<typename T> using add_lvalue_reference = std::add_lvalue_reference_t<T>;
template<typename T> using add_rvalue_reference = std::add_rvalue_reference_t<T>;
template<typename T> using add_pointer = std::add_pointer_t<T>;

template<typename T> using remove_const = std::remove_const_t<T>;
template<typename T> using remove_volatile = std::remove_volatile_t<T>;
template<typename T> using remove_cv = std::remove_cv_t<T>;
template<typename T> using remove_reference = std::remove_reference_t<T>;
template<typename T> using remove_cvref = std::remove_cvref_t<T>;
template<typename T> using remove_pointer = std::remove_pointer_t<T>;
template<typename T> using remove_extent = std::remove_extent_t<T>;
template<typename T> using remove_all_extents = std::remove_all_extents_t<T>;

template<typename F, typename...Args> using invoke_result = std::invoke_result_t<F, Args...>;

template<typename T> concept is_const = std::is_const_v<T>;
template<typename T> concept is_volatile = std::is_volatile_v<T>;
template<typename T> concept is_cv = is_const<T> && is_volatile<T>;
template<typename T> concept is_lvalue_reference = std::is_lvalue_reference_v<T>;
template<typename T> concept is_rvalue_reference = std::is_rvalue_reference_v<T>;
template<typename T> concept is_reference = is_lvalue_reference<T> || is_rvalue_reference<T>;
template<typename T> concept is_pointer = std::is_pointer_v<T>;
template<typename T> concept is_bounded_array = std::is_bounded_array_v<T>;
template<typename T> concept is_unbounded_array = std::is_unbounded_array_v<T>;
template<typename T> concept is_array = is_bounded_array<T> || is_unbounded_array<T>;

template<typename T, typename U> concept same_as = std::same_as<T, U>;
template<typename T, typename To> concept implicitly_convertible_to = std::is_convertible_v<T, To>;
template<typename T, typename To> concept nothrow_implicitly_convertible_to = implicitly_convertible_to<T, To> && std::is_nothrow_convertible_v<T, To>;
template<typename T, typename To> concept convertible_to = std::convertible_to<T, To> && implicitly_convertible_to<T, To>;
template<typename T, typename To> concept nothrow_convertible_to = convertible_to<T, To> && nothrow_implicitly_convertible_to<T, To> && requires(add_rvalue_reference<T>(&f)(void)noexcept) { { static_cast<To>(f()) }noexcept; };
template<typename T, typename Derived> concept is_base_of = std::is_base_of_v<T, Derived>;
template<typename T, typename Base> concept derived_from = std::derived_from<T, Base> && is_base_of<Base, T> && implicitly_convertible_to<const volatile T*, const volatile Base*>;

}}