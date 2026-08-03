#pragma once
#include <yw/string.h>
#include <yw/variant.h>

namespace yw {

template<char_type C> class null_terminated {
  static_assert(same_as<C, remove_cv<C>>);
  template<typename S> static constexpr bool _is_array = is_bounded_array<remove_ref<S>> && same_as<iter_value_t<S>, C>;
  variant<string<C>, string_view<C>> _data;

public:
  constexpr null_terminated() : _data(string_view<C>(empty_string<C>)) {}
  constexpr null_terminated(string<C>& str) : _data(string_view<C>(str)) {}
  constexpr null_terminated(const string<C>& str) : _data(string_view<C>(str)) {}
  constexpr null_terminated(string<C>&& str) : _data(std::move(str)) {}
  constexpr null_terminated(const string<C>&& str) : _data(std::move(str)) {}

  template<typename S> requires _is_array<S> constexpr null_terminated(const S& a)
    : _data(std::in_place_index_t<1>{}, string_view<C>(a, std::char_traits<C>::length(a))) {}

  template<stringable S> requires(!_is_array<S>) constexpr null_terminated(S&& s) : _data() {
    if constexpr (different_from<iter_value_t<S>, C>) _data.template emplace<0>(unicode<C>(static_cast<S&&>(s)));
    else _data.template emplace<0>(string<C>(string_view<C>(static_cast<S&&>(s))));
  }

  constexpr operator string_view<C>() const {
    switch (_data.index()) {
    case 0: return string_view<C>(_data.template get<0>());
    case 1: return string_view<C>(_data.template get<1>());
    default: return string_view<C>(empty_string<C>);
    }
  }

  constexpr bool empty() const noexcept {
    switch (_data.index()) {
    case 0: return _data.template get<0>().empty();
    case 1: return _data.template get<1>().empty();
    default: return true;
    }
  }

  constexpr size_t size() const noexcept {
    switch (_data.index()) {
    case 0: return _data.template get<0>().size();
    case 1: return _data.template get<1>().size();
    default: return 0;
    }
  }

  constexpr const C* data() const noexcept {
    switch (_data.index()) {
    case 0: return _data.template get<0>().data();
    case 1: return _data.template get<1>().data();
    default: return empty_string<C>.data();
    }
  }

  constexpr const C* c_str() const noexcept {
    switch (_data.index()) {
    case 0: return _data.template get<0>().c_str();
    case 1: return _data.template get<1>().c_str();
    default: return empty_string<C>.c_str();
    }
  }

  constexpr const C* begin() const noexcept { return data(); }

  constexpr const C* end() const noexcept {
    switch (_data.index()) {
    case 0: return _data.template get<0>().data() + _data.template get<0>().size();
    case 1: return _data.template get<1>().data() + _data.template get<1>().size();
    default: return empty_string<C>.data();
    }
  }

  constexpr string<C> to_string() const {
    switch (_data.index()) {
    case 0: return _data.template get<0>();
    case 1: return string<C>(_data.template get<1>());
    default: return string<C>();
    }
  }

  template<char_type C2> constexpr string<C2> to_string() const {
    switch (_data.index()) {
    case 0: return unicode<C2>(_data.template get<0>());
    case 1: return unicode<C2>(_data.template get<1>());
    default: return string<C2>();
    }
  }
};
} // namespace yw
