#pragma once
#include <core/array.h>
#include <core/heap.h>
#include <core/result.h>
#include <core/tuple.h>

namespace yw::errors {
inline constexpr error_type invalid_slotid{"invalid_slotid"};
inline constexpr error_type slot_creation_failed{"slot_creation_failed"};
} // namespace yw::errors

namespace yw {

///--------------------------------------------------------------------------///
/// MARK: slotset

template<typename T> class slotset {
  struct _slot {
    yw::heap<T> heap{};
    uint32_t generation = 1, next_free = uint32_t(-1);
  };

public:
  template<bool Const> class _iterator {
    friend class slotset;
    template<bool> friend class _iterator;
    using _owner_type = select_type<Const, const slotset*, slotset*>;
    _owner_type _p = nullptr;
    uint32_t _i = 0;
    constexpr _iterator(_owner_type owner, const uint32_t index) noexcept : _p(owner), _i(index) { _skip_empty(); }
    constexpr void _skip_empty() noexcept {
      if (!_p) return;
      while (_i < _p->_slots.size() && !_p->_slots[_i].heap) _i++;
    }

  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = select_type<Const, const T*, T*>;
    using reference = select_type<Const, const T&, T&>;
    constexpr _iterator() = default;
    template<bool C = Const> constexpr _iterator(const _iterator<false>& it) noexcept requires(C)
      : _p(it._p), _i(it._i) {}
    constexpr pointer operator->() const noexcept { return _p->_slots[_i].heap.operator->(); }
    constexpr reference operator*() const noexcept { return _p->_slots[_i].heap.operator*(); }
    constexpr _iterator& operator++() noexcept { return _i++, _skip_empty(), *this; }
    constexpr _iterator operator++(int) noexcept {
      const auto old = *this;
      return ++(*this), old;
    }
    template<bool C> constexpr bool operator==(const _iterator<C>& other) const noexcept {
      return _p == other._p && _i == other._i;
    }
  };

  using iterator = _iterator<false>;
  using const_iterator = _iterator<true>;

  struct slotid {
    using slotset_type = slotset<T>;
    uint32_t index{}, generation{};
    constexpr operator bool() const noexcept { return generation != 0; }
    friend constexpr bool operator==(const slotid a, const slotid b) noexcept = default;
    template<char_type C> constexpr string<C> to_string() const {
      return format<C>("slotid(index=", index, ", generation=", generation, ")");
    }
    constexpr string<char> to_string() const { return to_string<char>(); }
  };
  static_assert(sizeof(slotid) == 8);

private:
  array<_slot> _slots;
  uint32_t _free_head = uint32_t(-1);

public:
  constexpr ~slotset() noexcept = default;

  slotset(const slotset&) = delete;
  slotset& operator=(const slotset&) = delete;

  constexpr slotset() noexcept = default;

  constexpr bool exists(const slotid i) const noexcept { return get(i) != nullptr; }

  template<typename Self> constexpr auto get(this Self& self, const slotid i) noexcept {
    constexpr auto null_ptr = add_pointer<copy_cv<remove_ref<Self>, T>>{};
    if (i.index >= self._slots.size()) return null_ptr;
    auto& s = self._slots[i.index];
    const bool b = i.index < self._slots.size() && s.generation == i.generation;
    return b ? s.heap.get() : null_ptr;
  }

  constexpr result<void> erase(const slotid i) noexcept {
    if (!i) return {};
    if (i.index >= _slots.size()) return error(errors::invalid_slotid);
    if (auto& s = _slots[i.index]; s.generation == i.generation) {
      s.heap = {};
      s.generation++;
      s.next_free = _free_head;
      _free_head = i.index;
      return {};
    } else if (s.generation < i.generation) return error(errors::invalid_slotid);
    else return {};
  }

  template<typename... As> constexpr slotid emplace(As&&... as) noexcept {
    if (_free_head != uint32_t(-1)) {
      const auto i = _free_head;
      auto& s = _slots[i];
      _free_head = s.next_free;
      s.next_free = uint32_t(-1);
      s.heap = yw::heap<T>(static_cast<As&&>(as)...);
      return slotid{i, s.generation};
    } else {
      const auto i = uint32_t(_slots.size());
      _slots.push_back(_slot{yw::heap<T>(static_cast<As&&>(as)...), 1, uint32_t(-1)});
      return slotid{i, 1};
    }
  }

  constexpr void clear() {
    _free_head = uint32_t(-1);
    for (auto i = uint32_t(_slots.size()); i-- > 0;) {
      auto& s = _slots[i];
      s.heap = {};
      s.generation++;
      s.next_free = _free_head;
      _free_head = i;
    }
  }

  constexpr iterator begin() noexcept { return iterator(this, 0); }
  constexpr iterator end() noexcept { return iterator(this, uint32_t(_slots.size())); }
  constexpr const_iterator begin() const noexcept { return const_iterator(this, 0); }
  constexpr const_iterator end() const noexcept { return const_iterator(this, uint32_t(_slots.size())); }
};

} // namespace yw
