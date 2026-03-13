#pragma once
#include "yw/core.h"

namespace yw {

//////////////////////////////////////// MARK: slotset

template<typename T> class slotset {
public:
  struct slot {
    std::unique_ptr<T> pointer{};
    uint32_t generation = 1, next_free = uint32_t(-1);
  };

  struct slotid {
    using slot_type = T;
    using slotset_type = slotset<T>;
    uint32_t index{}, generation{};
    constexpr operator bool() const noexcept { return generation != 0; }
    friend constexpr bool operator==(const slotid a, const slotid b) noexcept = default;

    template<typename U> constexpr slotid& operator=(const U& other) noexcept requires requires {
      typename U::slot_type;
      typename U::slotset_type;
      requires specialization_of<typename U::slotset_type, slotset>;
      requires derived_from<T, typename U::slot_type>;
    } {
      index = other.index;
      generation = other.generation;
      return *this;
    }
  };
  static_assert(std::is_trivially_copyable_v<slotid>);
  static_assert(sizeof(slotid) == 2 * sizeof(uint32_t));

private:
  std::vector<slot> _slots;
  uint32_t _free_head = uint32_t(-1);

public:
  /// checks if the slotid is valid (i.e., the slot exists and has not been erased)
  bool contains(const slotid i) const noexcept {
    return i.index < _slots.size() && _slots[i.index].generation == i.generation;
  }

  /// returns a pointer to the slot if the slotid is valid, or nullptr otherwise
  T* get(const slotid i) noexcept {
    if (i.index >= _slots.size()) return nullptr;
    auto& s = _slots[i.index];
    return s.generation == i.generation ? s.pointer.get() : nullptr;
  }

  /// returns a const pointer to the slot if the slotid is valid, or nullptr otherwise
  const T* get(const slotid i) const noexcept {
    if (i.index >= _slots.size()) return nullptr;
    auto& s = _slots[i.index];
    return s.generation == i.generation ? s.pointer.get() : nullptr;
  }

  /// erases the slot with the given slotid if it is valid
  void erase(const slotid i) noexcept {
    if (i.index >= _slots.size()) return;
    if (auto& s = _slots[i.index]; s.generation == i.generation) {
      s.pointer.reset();
      s.generation++;
      s.next_free = _free_head;
      _free_head = i.index;
    }
  }

  /// creates a new slot with the given pointer and returns its slotid
  slotid add(std::unique_ptr<T> p) noexcept {
    try {
      if (_free_head != uint32_t(-1)) {
        const auto i = _free_head;
        auto& s = _slots[i];
        _free_head = s.next_free;
        s.pointer = std::move(p);
        s.next_free = uint32_t(-1);
        return slotid{i, s.generation};
      } else {
        const auto i = uint32_t(_slots.size());
        _slots.push_back(slot{std::move(p), 1, uint32_t(-1)});
        return slotid{i, 1};
      }
    } catch (...) { return {}; }
  }

  /// clears all slots
  void clear() {
    _slots.clear();
    _free_head = uint32_t(-1);
  }
};
} // namespace yw
