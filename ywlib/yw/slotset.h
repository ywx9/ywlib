#pragma once
#include "yw/error.h"
#include "yw/tuple.h"

namespace yw::errors {
inline constexpr error::kind invalid_slotid{"invalid slotid"};
inline constexpr error::kind slot_creation_failed{"slot creation failed"};
} // namespace yw::errors

namespace yw {

/// MARK: slotset

template<typename T> class slotset {
  struct _slot {
    std::unique_ptr<T> pointer{};
    uint32_t generation = 1, next_free = uint32_t(-1);
  };

public:
  template<bool Const> class _iterator {
    friend class slotset;
    template<bool> friend class _iterator;

    using _owner_type = select_type<Const, const slotset*, slotset*>;
    _owner_type _owner = nullptr;
    uint32_t _index = 0;
    _iterator(_owner_type owner, const uint32_t index) noexcept : _owner(owner), _index(index) { _skip_empty(); }
    void _skip_empty() noexcept {
      if (!_owner) return;
      while (_index < _owner->_slots.size() && !_owner->_slots[_index].pointer) _index++;
    }

  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = select_type<Const, const T*, T*>;
    using reference = select_type<Const, const T&, T&>;

    _iterator() = default;
    template<bool C = Const> _iterator(const _iterator<false>& it) noexcept requires(C)
      : _owner(it._owner), _index(it._index) {}

    reference operator*() const noexcept { return *_owner->_slots[_index].pointer; }
    pointer operator->() const noexcept { return _owner->_slots[_index].pointer.get(); }

    _iterator& operator++() noexcept {
      _index++;
      _skip_empty();
      return *this;
    }

    _iterator operator++(int) noexcept {
      const auto old = *this;
      ++(*this);
      return old;
    }

    template<bool A, bool B> friend bool operator==(const _iterator<A>& a, const _iterator<B>& b) noexcept {
      return a._owner == b._owner && a._index == b._index;
    }
  };

  using iterator = _iterator<false>;
  using const_iterator = _iterator<true>;

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
  std::vector<_slot> _slots;
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
  std::expected<void, error> erase(const slotid i) noexcept {
    if (i.index >= _slots.size()) return std::unexpected(error(errors::invalid_slotid));
    if (auto& s = _slots[i.index]; s.generation == i.generation) {
      s.pointer.reset();
      s.generation++;
      s.next_free = _free_head;
      _free_head = i.index;
      return {};
    } else if (s.generation < i.generation) return std::unexpected(error(errors::invalid_slotid));
    else return {}; // already erased
  }

  /// creates a new slot with the given pointer and returns its slotid
  slotid add(std::unique_ptr<T> p) noexcept {
    if (_free_head != uint32_t(-1)) {
      const auto i = _free_head;
      auto& s = _slots[i];
      _free_head = s.next_free;
      s.pointer = std::move(p);
      s.next_free = uint32_t(-1);
      return slotid{i, s.generation};
    } else {
      const auto i = uint32_t(_slots.size());
      _slots.push_back(_slot{std::move(p), 1, uint32_t(-1)});
      return slotid{i, 1};
    }
  }

  /// clears all slots
  void clear() {
    _slots.clear();
    _free_head = uint32_t(-1);
  }

  iterator begin() noexcept { return iterator(this, 0); }
  iterator end() noexcept { return iterator(this, uint32_t(_slots.size())); }
  const_iterator begin() const noexcept { return const_iterator(this, 0); }
  const_iterator end() const noexcept { return const_iterator(this, uint32_t(_slots.size())); }
};

/// MARK: general_handle

class general_handle {
public:
  struct slot {
    slotset<slot>::slotid id;
    yw::source_line source_line = yw::source_line::here();
    void clear() noexcept;
  };

protected:
  slotset<slot>::slotid _id{};
  explicit general_handle(slotset<slot>::slotid Id) : _id(Id) {}
  template<typename Handle> static typename Handle::slot* create_slot(const source_line& sl);
  template<typename Handle> static typename Handle::slot* get(slotset<slot>::slotid Id);

public:
  void clear() noexcept;

  ~general_handle() noexcept { clear(); }
  general_handle() noexcept = default;
  general_handle(const general_handle&) = delete;
  general_handle& operator=(const general_handle&) = delete;
  general_handle(general_handle&& Other) noexcept : _id(std::exchange(Other._id, {})) {}
  general_handle& operator=(general_handle&& Other) noexcept {
    if (this != &Other) {
      clear();
      _id = std::exchange(Other._id, {});
    }
    return *this;
  }

  slotset<slot>::slotid id() const noexcept { return _id; }
  explicit operator bool() const noexcept;
};

using general_slot = general_handle::slot;
using general_slotid = slotset<general_handle::slot>::slotid;

namespace internal {
inline slotset<general_handle::slot> general_slotset;
}

template<typename Handle> typename Handle::slot* general_handle::create_slot(const source_line& sl) {
  static_assert(derived_from<Handle, general_handle>);
  static_assert(derived_from<typename Handle::slot, general_handle::slot>);
  static_assert(constructible<Handle>); // default constructible
  const auto temp_id = internal::general_slotset.add(std::make_unique<typename Handle::slot>());
  const auto sp = general_handle::get<Handle>(temp_id);
  if (sp) sp->id = temp_id, sp->source_line = sl;
  else internal::general_slotset.erase(temp_id);
  return sp;
}

template<typename Handle> typename Handle::slot* general_handle::get(general_slotid Id) {
  static_assert(derived_from<Handle, general_handle>);
  static_assert(derived_from<typename Handle::slot, general_handle::slot>);
  return static_cast<typename Handle::slot*>(internal::general_slotset.get(Id));
}

inline void general_handle::slot::clear() noexcept { internal::general_slotset.erase(id); }
inline void general_handle::clear() noexcept { internal::general_slotset.erase(_id); }
inline general_handle::operator bool() const noexcept { return internal::general_slotset.contains(_id); }
} // namespace yw
