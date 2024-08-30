/// \file vector/function.hpp

#pragma once

#include "vector.hpp"

namespace yw::_ {

template<typename T> __m128 _vec(T&& x) noexcept {
  if constexpr (same_as<remove_cvref<T>, __m128>) return decay_copy(fwd<T>(x));
  else if constexpr (convertible_to<T, __m128>) return __m128(fwd<T>(x));
  else if constexpr (extent<T> == 4 && nt_tuple_for<T, fat4>)
    return _mm_setr_ps(get<0>(fwd<T>(x)), get<1>(fwd<T>(x)),
                       get<2>(fwd<T>(x)), get<3>(fwd<T>(x)));
  else if constexpr (extent<T> == 3 && nt_tuple_for<T, fat4>)
    return _mm_setr_ps(get<0>(fwd<T>(x)), get<1>(fwd<T>(x)),
                       get<2>(fwd<T>(x)), 0.f);
  else if constexpr (extent<T> == 2 && nt_tuple_for<T, fat4>)
    return _mm_setr_ps(get<0>(fwd<T>(x)), get<1>(fwd<T>(x)), 0.f, 0.f);
  else if constexpr (extent<T> == 1 && nt_tuple_for<T, fat4>)
    return _mm_set_ss(get<0>(fwd<T>(x)));
  else throw invalid_argument("invalid argument");
}

}

export namespace yw {


/// checks if the type can be handled as a vector
template<typename T> concept vectorizable =
  requires { _::_vec(declval<T>()); };


/// dot product of two 4D vectors
fat4 dot4(const vectorizable auto& a, const vectorizable auto& b)
  noexcept { return _mm_cvtss_f32(_mm_dp_ps(_::_vec(a), _::_vec(b), 0xf1)); }


/// dot product of two 3D vectors
fat4 dot3(const vectorizable auto& a, const vectorizable auto& b)
  noexcept { return _mm_cvtss_f32(_mm_dp_ps(_::_vec(a), _::_vec(b), 0x71)); }

/// dot product of two 2D vectors
fat4 dot2(const vectorizable auto& a, const vectorizable auto& b)
  noexcept { return _mm_cvtss_f32(_mm_dp_ps(_::_vec(a), _::_vec(b), 0x31)); }

/// cross product of two 3D vectors
vector cross(const vectorizable auto& a, const vectorizable auto& b) noexcept {
  return _mm_sub_ps(
    _mm_mul_ps(_mm_shuffle_ps(_::_vec(a), _::_vec(a), 0xc9),
               _mm_shuffle_ps(_::_vec(b), _::_vec(b), 0xd2)),
    _mm_mul_ps(_mm_shuffle_ps(_::_vec(a), _::_vec(a), 0xd2),
               _mm_shuffle_ps(_::_vec(b), _::_vec(b), 0xc9)));
}

/// length of a 4D vector
fat4 length4(const vectorizable auto& a) noexcept {
  auto b = _::_vec(a);
  return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(b, b, 0xf1)));
}

/// length of a 3D vector
fat4 length3(const vectorizable auto& a) noexcept {
  auto b = _::_vec(a);
  return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(b, b, 0x71)));
}

/// length of a 2D vector
fat4 length2(const vectorizable auto& a) noexcept {
  auto b = _::_vec(a);
  return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(b, b, 0x31)));
}

/// normalizes a 4D vector
vector normalize4(const vectorizable auto& a) noexcept {
  auto b = _::_vec(a);
  return _mm_mul_ps(b, _mm_rsqrt_ps(_mm_dp_ps(b, b, 0xff)));
}

/// normalizes a 3D vector
vector normalize3(const vectorizable auto& a) noexcept {
  auto b = _::_vec(a);
  return _mm_mul_ps(b, _mm_rsqrt_ps(_mm_dp_ps(b, b, 0x7f)));
}

/// normalizes a 2D vector
vector normalize2(const vectorizable auto& a) noexcept {
  auto b = _::_vec(a);
  return _mm_mul_ps(b, _mm_rsqrt_ps(_mm_dp_ps(b, b, 0x3f)));
}

///





} ////////////////////////////////////////////////////////////////////////////// namespace yw
