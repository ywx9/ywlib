#pragma once
#include <core/core.h>
#include <core/result.h>

namespace yw {

class any {
  union alignas(std::max_align_t) _union {
    none _none;
    std::byte _sbo[32];
    void* _heap;
    constexpr ~_union() noexcept = default;
    constexpr _union() noexcept : _none() {}
    constexpr void* _get_sbo() noexcept { return static_cast<void*>(_sbo); }
    constexpr const void* _get_sbo() const noexcept { return static_cast<const void*>(_sbo); }
  } _union{};
  static_assert(sizeof(_union) == 32);

  struct _traits {
    void (*_destroy)(decltype(_union)& src) = nullptr;
    void (*_copy_to)(const decltype(_union)& src, decltype(_union)& dst) = nullptr;
    void (*_move_to)(decltype(_union)&& src, decltype(_union)& dst) = nullptr;
    const void* (*_type_id)() = nullptr;
    constexpr void reset() noexcept { _destroy = nullptr, _copy_to = nullptr, _move_to = nullptr, _type_id = nullptr; }
  } _traits{};

  template<typename T> static constexpr bool _is_small_object =
    sizeof(T) <= sizeof(_union) && alignof(T) <= alignof(_union);

  template<typename T> struct _traits_small : _traits {
    static constexpr int tag{};
    constexpr _traits_small() noexcept {
      _traits::_destroy = [](decltype(_union)& src) noexcept { reinterpret_cast<T*>(src._get_sbo())->~T(); };
      if constexpr (constructible<T, const T&>) {
        _traits::_copy_to = [](const decltype(_union)& src, decltype(_union)& dst) {
          new (dst._get_sbo()) T(*reinterpret_cast<const T*>(src._get_sbo()));
        };
      }
      _traits::_move_to = [](decltype(_union)&& src, decltype(_union)& dst) {
        new (dst._get_sbo()) T(static_cast<T&&>(*reinterpret_cast<T*>(src._get_sbo())));
        reinterpret_cast<T*>(src._get_sbo())->~T();
      };
      _traits::_type_id = []() -> const void* { return static_cast<const void*>(&tag); };
    }
  };

  template<typename T> struct _traits_large : _traits {
    static constexpr int tag{};
    constexpr _traits_large() noexcept {
      _traits::_destroy = [](decltype(_union)& src) noexcept {
        static_cast<T*>(src._heap)->~T();
        yw::deallocate(static_cast<T*>(src._heap));
      };
      if constexpr (constructible<T, const T&>) {
        _traits::_copy_to = [](const decltype(_union)& src, decltype(_union)& dst) {
          dst._heap = allocate<T>();
          new (dst._heap) T(*static_cast<const T*>(src._heap));
        };
      }
      _traits::_move_to = [](decltype(_union)&& src, decltype(_union)& dst) {
        dst._heap = src._heap;
        src._heap = nullptr;
      };
      _traits::_type_id = []() -> const void* { return static_cast<const void*>(&tag); };
    }
  };

  template<typename T, typename... As>
  constexpr void _emplace(As&&... as) noexcept(nt_constructible<remove_cvref<T>, As...>) {
    using G = remove_cvref<T>;
    if constexpr (_is_small_object<G>) {
      new (_union._get_sbo()) G(static_cast<As&&>(as)...);
      new (&_traits) _traits_small<G>();
    } else {
      _union._heap = allocate<G>();
      new (_union._heap) G(static_cast<As&&>(as)...);
      new (&_traits) _traits_large<G>();
    }
  }

public:
  constexpr ~any() noexcept {
    if (_traits._destroy) _traits._destroy(_union);
  }

  constexpr any() noexcept = default;

  constexpr any(const any& o) {
    if (!o._traits._copy_to) return;
    o._traits._copy_to(o._union, _union);
    _traits = o._traits;
  }

  constexpr any(any&& o) {
    if (o._traits._move_to) o._traits._move_to(move(o._union), _union);
    _traits = o._traits;
    o._traits.reset();
  }

  constexpr any& operator=(const any& o) {
    if (this != &o) {
      if (_traits._destroy) _traits._destroy(_union);
      if (o._traits._copy_to) {
        o._traits._copy_to(o._union, _union);
        _traits = o._traits;
      } else _traits.reset();
    }
    return *this;
  }

  constexpr any& operator=(any&& o) {
    if (this != &o) {
      if (_traits._destroy) _traits._destroy(_union);
      if (o._traits._move_to) o._traits._move_to(move(o._union), _union);
      _traits = o._traits;
      o._traits.reset();
    }
    return *this;
  }

  template<typename T> constexpr any(T&& t) noexcept(nt_constructible<remove_cvref<T>, T>) {
    using G = remove_cvref<T>;
    _emplace<G>(static_cast<T&&>(t));
  }

  template<typename T> constexpr any& operator=(T&& t) noexcept(nt_constructible<remove_cvref<T>, T>) {
    if (_traits._destroy) _traits._destroy(_union);
    using G = remove_cvref<T>;
    _emplace<G>(static_cast<T&&>(t));
    return *this;
  }

  template<typename T, typename... As>
  constexpr void emplace(As&&... as) noexcept(nt_constructible<remove_cvref<T>, As...>) {
    if (_traits._destroy) _traits._destroy(_union);
    using G = remove_cvref<T>;
    _emplace<G>(static_cast<As&&>(as)...);
  }

  constexpr void reset() noexcept {
    if (_traits._destroy) _traits._destroy(_union);
    _traits.reset();
  }

  constexpr bool has_value() const noexcept { return _traits._type_id; }

  template<typename T> constexpr bool is() const noexcept {
    using G = remove_cvref<T>;
    if constexpr (_is_small_object<G>)
      return _traits._type_id && _traits._type_id() == static_cast<void*>(&_traits_small<G>::tag);
    else return _traits._type_id && _traits._type_id() == static_cast<void*>(&_traits_large<G>::tag);
  }

  template<typename T> constexpr T& as() & noexcept {
    using G = remove_cvref<T>;
    if (!is<G>()) error("bad any cast").print_and_abort();
    if constexpr (_is_small_object<G>) return *reinterpret_cast<G*>(_union._get_sbo());
    else return *static_cast<G*>(_union._heap);
  }
};
} // namespace yw
