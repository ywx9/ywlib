#pragma once
#include "yw/core.h"

namespace yw {

constexpr size_t function_sbo_size = 24;

template<typename R, typename... As> class function {
  static constexpr size_t _sbo_size = function_sbo_size;
  alignas(std::max_align_t) std::byte _storage[_sbo_size]{};

  // function pointer route
  R (*fp)(As...) = nullptr;

  // function object route (SBO only)
  R (*_invoke)(const void* src, As&&... args) = nullptr;
  void (*_destroy)(void* src) = nullptr;
  void (*_move)(void* dst, void* src) = nullptr;
  void (*_copy)(void* dst, const void* src) = nullptr;

  bool _has_obj() const noexcept { return _destroy != nullptr; }

  void _reset() noexcept {
    if (_has_obj()) _destroy(_storage);
    fp = nullptr;
    _invoke = nullptr;
    _destroy = nullptr;
    _move = nullptr;
    _copy = nullptr;
  }

public:
  constexpr function() noexcept = default;

  // assigns a function pointer
  template<class F> //
  requires is_pointer<remove_ref<F>> && is_function<remove_pointer<remove_ref<F>>> && invocable_r<F, R, As...>
  constexpr function(F f) noexcept : fp(f) {}

  // assigns a function object (SBO only, copyable)
  template<class F> //
  requires invocable_r<const remove_cvref<F>&, R, As...> &&
           (!(is_pointer<remove_ref<F>> && is_function<remove_pointer<remove_ref<F>>>)) &&
           (sizeof(remove_cvref<F>) <= _sbo_size) && (alignof(remove_cvref<F>) <= alignof(std::max_align_t)) &&
           std::is_nothrow_move_constructible_v<remove_cvref<F>> && std::is_copy_constructible_v<remove_cvref<F>> &&
           different_from<remove_cvref<F>, function> function(F&& f) noexcept {
    using G = remove_cvref<F>;
    new (_storage) G(static_cast<F&&>(f));
    _invoke = [](const void* src, As&&... args) -> R {
      return (*static_cast<const G*>(src))(static_cast<As&&>(args)...);
    };
    _destroy = [](void* src) { static_cast<G*>(src)->~G(); };
    _move = [](void* dst, void* src) {
      new (dst) G(std::move(*static_cast<G*>(src)));
      static_cast<G*>(src)->~G();
    };
    _copy = [](void* dst, const void* src) { new (dst) G(*static_cast<const G*>(src)); };
  }

  function(const function& other) {
    fp = other.fp;
    _invoke = other._invoke;
    _destroy = other._destroy;
    _move = other._move;
    _copy = other._copy;
    if (other._has_obj()) other._copy(_storage, other._storage);
  }

  function(function&& other) noexcept {
    fp = other.fp;
    _invoke = other._invoke;
    _destroy = other._destroy;
    _move = other._move;
    _copy = other._copy;
    if (other._has_obj()) other._move(_storage, other._storage);
    other.fp = nullptr;
    other._invoke = nullptr;
    other._destroy = nullptr;
    other._move = nullptr;
    other._copy = nullptr;
  }

  function& operator=(const function& other) {
    if (this == &other) return *this;
    _reset();
    fp = other.fp;
    _invoke = other._invoke;
    _destroy = other._destroy;
    _move = other._move;
    _copy = other._copy;
    if (other._has_obj()) other._copy(_storage, other._storage);
    return *this;
  }

  function& operator=(function&& other) noexcept {
    if (this == &other) return *this;
    _reset();
    fp = other.fp;
    _invoke = other._invoke;
    _destroy = other._destroy;
    _move = other._move;
    _copy = other._copy;
    if (other._has_obj()) other._move(_storage, other._storage);
    other.fp = nullptr;
    other._invoke = nullptr;
    other._destroy = nullptr;
    other._move = nullptr;
    other._copy = nullptr;
    return *this;
  }

  ~function() { _reset(); }

  R operator()(As... args) const {
    if (fp) return fp(std::forward<As>(args)...);
    if (!_invoke) throw std::bad_function_call();
    return _invoke(_storage, std::forward<As>(args)...);
  }

  explicit operator bool() const noexcept { return fp || _invoke; }

  void reset() noexcept { _reset(); }

  class slot {
    function* _dest = nullptr;
    slot& operator=(const slot&) & = delete;

  public:
    explicit slot() noexcept = default;
    slot(function& Dest) noexcept : _dest(&Dest) {}

    void operator=(function func) && {
      if (_dest) *_dest = std::move(func);
    }

    void operator=(const slot& func) && {
      if (_dest && func._dest) *_dest = *func._dest;
    }

    void reset() && {
      if (_dest) _dest->reset();
    }
  };
};
} // namespace yw
