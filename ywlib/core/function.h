#pragma once
#include <core/core.h>
#include <core/result.h>

namespace yw {

enum class function_type { none, pointer, heap, sbo };

template<typename R, typename... As> class function {
  union alignas(std::max_align_t) _union {
    none _none;
    R (*_pointer)(As...);
    std::byte _sbo[32];
    void* _heap;
    constexpr ~_union() noexcept = default;
    constexpr _union() noexcept : _none() {}
  } _union{};
  static_assert(sizeof(_union) == 32);
  struct _traits {
    void (*_destroy)(decltype(_union)& src) = nullptr;
    void (*_copy_to)(const decltype(_union)& src, decltype(_union)& dst) = nullptr;
    void (*_move_to)(decltype(_union)&& src, decltype(_union)& dst) = nullptr;
    R (*_invoke)(decltype(_union)& src, As&&... as) = nullptr;
    constexpr void reset() noexcept { _destroy = nullptr, _copy_to = nullptr, _move_to = nullptr, _invoke = nullptr; }
  } _traits{};

  constexpr function_type _get_function_type() const noexcept {
    if (_traits._invoke) return _traits._destroy ? function_type::sbo : function_type::heap;
    else return _traits._copy_to ? function_type::pointer : function_type::none;
  }

  template<function_type T> constexpr void _destroy() noexcept {
    if constexpr (T == function_type::heap) deallocate(_union._heap);
    else if constexpr (T == function_type::sbo) _traits._destroy(_union);
  }

  template<function_type T> constexpr void _copy_from(const function& src) noexcept {
    if constexpr (T == function_type::pointer) _traits._copy_to(src._union, _union);
    else if constexpr (T == function_type::heap) _traits._copy_to(src._union, _union);
    else if constexpr (T == function_type::sbo) _traits._copy_to(src._union, _union);
  }

  template<function_type T> constexpr void _move_from(function&& src) noexcept {
    if constexpr (T == function_type::pointer) _traits._move_to(move(src._union), _union);
    else if constexpr (T == function_type::heap) _traits._move_to(move(src._union), _union);
    else if constexpr (T == function_type::sbo) _traits._move_to(move(src._union), _union);
  }

public:
  using result_type = select_type<
    inspect(specialization_of<remove_cvref<R>, result>, specialization_of<remove_cvref<R>, yw::optional>), R,
    result<typename R::value_type>, result<remove_cvref<R>>>;

  constexpr ~function() noexcept {
    if (const auto t = _get_function_type(); t == function_type::heap) deallocate(_union._heap);
    else if (t == function_type::sbo) _traits._destroy(_union);
  }

  constexpr function() noexcept = default;

  constexpr function(const function& o) noexcept {
    if (o._traits._copy_to) o._traits._copy_to(o._union, _union);
    _traits = o._traits;
  }

  constexpr function(function&& o) noexcept {
    if (o._traits._move_to) o._traits._move_to(move(o._union), _union);
    _traits = o._traits;
    o._traits.reset();
  }

  constexpr function& operator=(const function& o) noexcept {
    if (this != &o) {
      if (_traits._destroy) _traits._destroy(_union);
      if (o._traits._copy_to) o._traits._copy_to(o._union, _union);
      _traits = o._traits;
    }
    return *this;
  }

  constexpr function& operator=(function&& o) noexcept {
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
    using ftype = R (*)(As...);
    struct traits : _traits {
      constexpr traits() noexcept {
        _traits::_copy_to = [](const decltype(_union)& src, decltype(_union)& dst) {
          new (&dst._pointer) ftype(src._pointer);
        };
        _traits::_move_to = [](decltype(_union)&& src, decltype(_union)& dst) {
          new (&dst._pointer) ftype(src._pointer);
          src._pointer = nullptr;
        };
      }
    };
    new (&_union._pointer) ftype(f);
    new (&_traits) traits();
  }

  /// construct from a large function object (use heap allocation)
  template<invocable_r<R, As...> F> requires(sizeof(F) > sizeof(_union) && !is_pointer<F>)
  constexpr function(F&& f) noexcept {
    struct traits : _traits {
      constexpr traits() noexcept {
        _traits::_copy_to = [](const decltype(_union)& src, decltype(_union)& dst) {
          dst._heap = allocate<F>();
          new (dst._heap) F(*static_cast<const F*>(src._heap));
        };
        _traits::_move_to = [](decltype(_union)&& src, decltype(_union)& dst) {
          dst._heap = src._heap;
          src._heap = nullptr;
        };
        _traits::_invoke = [](decltype(_union)& src, As&&... as) -> R {
          return static_cast<R>((*static_cast<F*>(src._heap))(static_cast<As&&>(as)...));
        };
      }
    };
    _union._heap = allocate<F>();
    new (_union._heap) F(static_cast<F&&>(f));
    new (&_traits) traits();
  }

  /// construct from a small function object
  template<invocable_r<R, As...> F> requires(sizeof(F) <= sizeof(_union) && !is_pointer<F>)
  constexpr function(F&& f) noexcept {
    struct traits : _traits {
      constexpr traits() noexcept {
        _traits::_destroy = [](decltype(_union)& src) noexcept { static_cast<F*>(src._sbo)->~F(); };
        _traits::_copy_to = [](const decltype(_union)& src, decltype(_union)& dst) { new (&dst._sbo) F(src._sbo); };
        _traits::_move_to = [](decltype(_union)&& src, decltype(_union)& dst) {
          new (&dst._sbo) F(static_cast<F&&>(src._sbo));
          static_cast<F*>(src._sbo)->~F();
        };
        _traits::_invoke = [](decltype(_union)& src, As&&... as) -> R {
          return static_cast<R>(src._sbo(static_cast<As&&>(as)...));
        };
      }
    };
    new (&_union._sbo) F(static_cast<F&&>(f));
    new (&_traits) traits();
  }

  constexpr result_type operator()(As&&... as) noexcept {
    switch (_get_function_type()) {
    case function_type::pointer: return _union._pointer(static_cast<As&&>(as)...);
    case function_type::heap: return _traits._invoke(_union, static_cast<As&&>(as)...);
    case function_type::sbo: return _traits._invoke(_union, static_cast<As&&>(as)...);
    default: return error("uninitialized function");
    }
  }
};
} // namespace yw
