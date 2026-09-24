#pragma once
#include <core/core.h>
#include <core/result.h>

namespace yw {

template<typename R, typename... As> class function {
  using _result_type = select_type<is_void<R>, none, R>;

  union alignas(std::max_align_t) _union {
    none _none;
    R (*_pointer)(As...);
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
    _result_type (*_invoke)(decltype(_union)& src, As&&... as) = nullptr;
    constexpr void reset() noexcept { _destroy = nullptr, _copy_to = nullptr, _move_to = nullptr, _invoke = nullptr; }
  } _traits{};

public:
  constexpr ~function() noexcept {
    if (_traits._destroy) _traits._destroy(_union);
  }

  constexpr function() noexcept = default;

  constexpr function(const function& o) {
    if (!o._traits._copy_to) return;
    o._traits._copy_to(o._union, _union);
    _traits = o._traits;
  }

  constexpr function(function&& o) {
    if (o._traits._move_to) o._traits._move_to(move(o._union), _union);
    _traits = o._traits;
    o._traits.reset();
  }

  constexpr function& operator=(const function& o) {
    if (this != &o) {
      if (_traits._destroy) _traits._destroy(_union);
      if (o._traits._copy_to) {
        o._traits._copy_to(o._union, _union);
        _traits = o._traits;
      } else _traits.reset();
    }
    return *this;
  }

  constexpr function& operator=(function&& o) {
    if (this != &o) {
      if (_traits._destroy) _traits._destroy(_union);
      if (o._traits._move_to) o._traits._move_to(move(o._union), _union);
      _traits = o._traits;
      o._traits.reset();
    }
    return *this;
  }

  /// construct from a function pointer
  template<is_pointer F> requires is_function<remove_pointer<F>> && invocable_r<F, R, As...>
  constexpr function(F f) noexcept {
    if (!f) return;
    using ftype = R (*)(As...);
    struct traits : _traits {
      constexpr traits() noexcept {
        _traits::_destroy = [](decltype(_union)& src) noexcept { src._pointer = nullptr; };
        _traits::_copy_to = [](const decltype(_union)& src, decltype(_union)& dst) {
          new (&dst._pointer) ftype(src._pointer);
        };
        _traits::_move_to = [](decltype(_union)&& src, decltype(_union)& dst) {
          new (&dst._pointer) ftype(src._pointer);
          src._pointer = nullptr;
        };
        _traits::_invoke = [](decltype(_union)& src, As&&... as) -> _result_type {
          return yw::invoke(src._pointer, static_cast<As&&>(as)...);
        };
      }
    };
    new (&_union._pointer) ftype(f);
    new (&_traits) traits();
  }

  /// construct from a large function object (use heap allocation)
  template<invocable_r<R, As...> F> requires(sizeof(F) > sizeof(_union) && !is_pointer<F> && !is_function<F>)
  constexpr function(F&& f) noexcept(nt_constructible<remove_cvref<F>, F>) {
    using G = remove_cvref<F>;
    using H = char[sizeof(G)];
    struct traits : _traits {
      constexpr traits() noexcept {
        _traits::_destroy = [](decltype(_union)& src) noexcept {
          static_cast<G*>(src._heap)->~G();
          yw::deallocate(static_cast<H*>(src._heap));
        };
        if constexpr (constructible<F, const F&>) {
          _traits::_copy_to = [](const decltype(_union)& src, decltype(_union)& dst) {
            dst._heap = allocate<G>();
            new (dst._heap) G(*static_cast<const G*>(src._heap));
          };
        }
        _traits::_move_to = [](decltype(_union)&& src, decltype(_union)& dst) {
          dst._heap = src._heap;
          src._heap = nullptr;
        };
        _traits::_invoke = [](decltype(_union)& src, As&&... as) -> _result_type {
          return yw::invoke(*static_cast<G*>(src._heap), static_cast<As&&>(as)...);
        };
      }
    };
    _union._heap = allocate<G>();
    new (_union._heap) G(static_cast<F&&>(f));
    new (&_traits) traits();
  }

  /// construct from a small function object
  template<invocable_r<R, As...> F> requires(
    sizeof(remove_cvref<F>) <= sizeof(_union) && alignof(remove_cvref<F>) <= alignof(_union) && !is_pointer<F> &&
    !is_function<F>) constexpr function(F&& f) noexcept(nt_constructible<remove_cvref<F>, F>) {
    using G = remove_cvref<F>;
    struct traits : _traits {
      constexpr traits() noexcept {
        _traits::_destroy = [](decltype(_union)& src) noexcept { reinterpret_cast<G*>(src._get_sbo())->~G(); };
        if constexpr (constructible<G, const G&>) {
          _traits::_copy_to = [](const decltype(_union)& src, decltype(_union)& dst) {
            new (dst._get_sbo()) G(*reinterpret_cast<const G*>(src._get_sbo()));
          };
        }
        _traits::_move_to = [](decltype(_union)&& src, decltype(_union)& dst) {
          new (dst._get_sbo()) G(static_cast<G&&>(*reinterpret_cast<G*>(src._get_sbo())));
          reinterpret_cast<G*>(src._get_sbo())->~G();
        };
        _traits::_invoke = [](decltype(_union)& src, As&&... as) -> _result_type {
          return yw::invoke(*reinterpret_cast<G*>(src._get_sbo()), static_cast<As&&>(as)...);
        };
      }
    };
    new (&_union._sbo) G(static_cast<F&&>(f));
    new (&_traits) traits();
  }

  constexpr R operator()(As&&... as) {
    if (!_traits._invoke) error("uninitialized function").print_and_abort();
    if constexpr (is_void<R>) _traits._invoke(_union, static_cast<As&&>(as)...);
    else return _traits._invoke(_union, static_cast<As&&>(as)...);
  }
};

static_assert(sizeof(function<void>) == 64);
} // namespace yw
