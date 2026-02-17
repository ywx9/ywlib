#pragma once
#include "yw/core.h"

namespace yw {

//////////////////////////////////////// MARK: slotlist

template<typename T, typename Del = std::default_delete<T>> class slotlist {
public:
  struct id {
    uint32_t index{}, generation{};

    constexpr id() noexcept = default;
    constexpr id(const id&) = default;
    constexpr id& operator=(const id&) = default;
    constexpr id(uint32_t i, uint32_t g) noexcept : index(i), generation(g) {}

    /// \note ほとんどのシステムで、idはユニークであることが期待されるため、被ムーブオブジェクトはゼロ初期化する。

    constexpr id(id&& Id) noexcept : index(std::exchange(Id.index, 0)), generation(std::exchange(Id.generation, 0)) {}
    constexpr id& operator=(id&& Id) noexcept {
      index = std::exchange(Id.index, 0);
      generation = std::exchange(Id.generation, 0);
      return *this;
    }

    friend constexpr bool operator==(const id& a, const id& b) noexcept = default;
    constexpr bool is_zero() const noexcept { return (index | generation) == 0; }
  };
  static_assert(sizeof(id) == 2 * sizeof(uint32_t));

  struct slot {
    std::unique_ptr<T, Del> pointer{};
    uint32_t generation = 1, next_free = uint32_t(-1);
  };

private:
  struct _id {
    uint32_t index{}, generation{};
    friend constexpr bool operator==(const _id& a, const _id& b) noexcept = default;
  };
  static_assert(std::is_trivially_copyable_v<_id> && sizeof(_id) == sizeof(id));

  std::vector<slot> _slots;
  std::vector<_id> _list;
  uint32_t _free_head = uint32_t(-1);

  id _push(std::unique_ptr<T, Del> p) {
    if (_free_head != uint32_t(-1)) {
      const auto i = _free_head;
      auto& s = _slots[i];
      _free_head = s.next_free;
      s.pointer = std::move(p);
      s.next_free = uint32_t(-1);
      return id{i, s.generation};
    } else {
      const auto i = uint32_t(_slots.size());
      _slots.push_back(slot{std::move(p), 1, uint32_t(-1)});
      return id{i, 1};
    }
  }

  T* get(const _id& i) noexcept {
    if (i.index >= _slots.size()) return nullptr;
    if (auto& s = _slots[i.index]; s.generation != i.generation) return nullptr;
    else return s.pointer.get();
  }

  const T* get(const _id& i) const noexcept {
    if (i.index >= _slots.size()) return nullptr;
    if (auto& s = _slots[i.index]; s.generation != i.generation) return nullptr;
    else return s.pointer.get();
  }

public:

  bool contains(const id& i) const noexcept { return i.index < _slots.size() && _slots[i.index].generation == i.generation; }

  T* get(const id& i) noexcept {
    if (i.index >= _slots.size()) return nullptr;
    if (auto& s = _slots[i.index]; s.generation != i.generation) return nullptr;
    else return s.pointer.get();
  }

  const T* get(const id& i) const noexcept {
    if (i.index >= _slots.size()) return nullptr;
    if (auto& s = _slots[i.index]; s.generation != i.generation) return nullptr;
    else return s.pointer.get();
  }

  void erase(const id& i) {
    if (i.index >= _slots.size()) return;
    if (auto& s = _slots[i.index]; s.generation == i.generation) {
      s.pointer.reset();
      s.generation++;
      s.next_free = _free_head;
      _free_head = i.index;
      if (auto it = std::ranges::find(_list, _id{i.index, i.generation}); it != _list.end()) _list.erase(it);
    }
  }

  id push(std::unique_ptr<T, Del> p) {
    const auto i = _push(std::move(p));
    _list.emplace_back(i.index, i.generation);
    return i;
  }

  void clear() {
    _slots.clear();
    _list.clear();
    _free_head = uint32_t(-1);
  }

  void set_order(const id& i, size_t pos) {
    if (pos < _list.size()) {
      if (auto it = std::ranges::find(_list, _id{i.index, i.generation}); it != _list.end()) {
        size_t from = size_t(it - _list.begin());
        if (pos < from) {
          std::memmove(_list.data() + pos + 1, _list.data() + pos, (from - pos) * sizeof(_id));
          _list[pos] = _id{i.index, i.generation};
        } else if (pos > from) {
          std::memmove(_list.data() + from, _list.data() + from + 1, (pos - from) * sizeof(_id));
          _list[pos - 1] = _id{i.index, i.generation};
        }
      }
    } else bring_to_last(i);
  }

  void bring_to_first(const id& i) {
    if (auto it = std::ranges::find(_list, _id{i.index, i.generation}); it != _list.end()) {
      size_t from = size_t(it - _list.begin());
      std::memmove(_list.data() + 1, _list.data(), from * sizeof(_id));
      _list[0] = _id{i.index, i.generation};
    }
  }

  void bring_to_last(const id& i) {
    if (auto it = std::ranges::find(_list, _id{i.index, i.generation}); it != _list.end()) {
      size_t from = size_t(it - _list.begin());
      std::memmove(_list.data() + from, _list.data() + from + 1, (_list.size() - from - 1) * sizeof(_id));
      _list[_list.size() - 1] = _id{i.index, i.generation};
    }
  }

  class iterator {
    slotlist<T, Del>* _list;
    size_t _pos;
  public:
    using iterator_category = std::random_access_iterator_tag;
    using iterator_concept = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;

    iterator() : _list(nullptr), _pos(0) {}
    iterator(slotlist<T, Del>* list, size_t pos) : _list(list), _pos(pos) {}
    T& operator*() const { return *(_list->_slots[_list->_list[_pos].index].pointer); }
    T* operator->() const { return _list->_slots[_list->_list[_pos].index].pointer.get(); }
    T& operator[](size_t i) const { return *(_list->_slots[_list->_list[_pos + i].index].pointer); }
    auto& operator++() { ++_pos; return *this; }
    auto operator++(int) { iterator temp = *this; ++_pos; return temp; }
    auto& operator--() { --_pos; return *this; }
    auto operator--(int) { iterator temp = *this; --_pos; return temp; }
    auto& operator+=(difference_type n) { _pos += n; return *this; }
    auto& operator-=(difference_type n) { _pos -= n; return *this; }
    auto operator+(difference_type n) const { return iterator(_list, _pos + n); }
    auto operator-(difference_type n) const { return iterator(_list, _pos - n); }
    auto operator-(const iterator& i) const { return difference_type(_pos) - difference_type(i._pos); }
    friend auto operator+(difference_type n, const iterator& i) { return iterator(i._list, i._pos + n); }
    bool operator==(const iterator& i) const { return _pos == i._pos; }
    auto operator<=>(const iterator& i) const { return _pos <=> i._pos; }
  };

  class const_iterator {
    const slotlist<T, Del>* _list;
    size_t _pos;
  public:
    using iterator_category = std::random_access_iterator_tag;
    using iterator_concept = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;

    const_iterator() : _list(nullptr), _pos(0) {}
    const_iterator(const slotlist<T, Del>* list, size_t pos) : _list(list), _pos(pos) {}
    const T& operator*() const { return *(_list->_slots[_list->_list[_pos].index].pointer); }
    const T* operator->() const { return _list->_slots[_list->_list[_pos].index].pointer.get(); }
    const T& operator[](difference_type n) const { return *(_list->_slots[_list->_list[_pos + n].index].pointer); }
    auto& operator++() { ++_pos; return *this; }
    auto operator++(int) { const_iterator temp = *this; ++_pos; return temp; }
    auto& operator--() { --_pos; return *this; }
    auto operator--(int) { const_iterator temp = *this; --_pos; return temp; }
    auto& operator+=(difference_type n) { _pos += n; return *this; }
    auto& operator-=(difference_type n) { _pos -= n; return *this; }
    auto operator+(difference_type n) const { return const_iterator(_list, _pos + n); }
    auto operator-(difference_type n) const { return const_iterator(_list, _pos - n); }
    auto operator-(const const_iterator& i) const { return difference_type(_pos) - difference_type(i._pos); }
    friend auto operator+(difference_type n, const const_iterator& i) { return const_iterator(i._list, i._pos + n); }
    bool operator==(const const_iterator& i) const { return _pos == i._pos; }
    auto operator<=>(const const_iterator& i) const { return _pos <=> i._pos; }
  };

  auto begin() noexcept { return iterator(this, 0); }
  auto begin() const noexcept { return const_iterator(this, 0); }
  auto end() noexcept { return iterator(this, _list.size()); }
  auto end() const noexcept { return const_iterator(this, _list.size()); }
  auto size() const noexcept { return _list.size(); }
  auto empty() const noexcept { return _list.empty(); }
  auto& operator[](size_t i) noexcept { return *get(_list[i]); }
  auto& operator[](size_t i) const noexcept { return *get(_list[i]); }
};
} // namespace yw
