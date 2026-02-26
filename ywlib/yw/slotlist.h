#pragma once
#include "yw/core.h"

namespace yw {

//////////////////////////////////////// MARK: slotid

struct slotid {
  uint32_t index{}, generation{};
  constexpr operator bool() const noexcept { return generation != 0; }
  friend constexpr bool operator==(const slotid& a, const slotid& b) noexcept = default;
};

static_assert(std::is_trivially_copyable_v<slotid>);
static_assert(sizeof(slotid) == 2 * sizeof(uint32_t));

//////////////////////////////////////// MARK: slotlist

template<typename T, typename Del = std::default_delete<T>> class slotlist {
public:
  struct slot {
    std::unique_ptr<T, Del> pointer{};
    uint32_t generation = 1, next_free = uint32_t(-1);
  };

private:
  std::vector<slot> _slots;
  std::vector<slotid> _list;
  uint32_t _free_head = uint32_t(-1);

  slotid _push(std::unique_ptr<T, Del> p) {
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
  }

  void _erase(uint32_t index) {
    const auto p = _list.data();
    const auto n = _list.size();
    if (index == 0) std::memmove(p, p + 1, (n - 1) * sizeof(slotid));
    else if (index < n - 1) std::memmove(p + index, p + index + 1, (n - index - 1) * sizeof(slotid));
    _list.pop_back();
  }

  void _insert(size_t from, size_t to) {
    const auto p = _list.data();
    if (from < to) {
      std::memmove(p + from, p + from + 1, (to - from) * sizeof(slotid));
      std::memcpy(p + to, p + from, sizeof(slotid));
    } else if (from > to) {
      std::memmove(p + to + 1, p + to, (from - to) * sizeof(slotid));
      std::memcpy(p + to, p + from + 1, sizeof(slotid));
    }
  }

public:
  bool contains(const slotid& i) const noexcept {
    return i.index < _slots.size() && _slots[i.index].generation == i.generation;
  }

  T* get(const slotid& i) noexcept {
    if (i.index >= _slots.size()) return nullptr;
    auto& s = _slots[i.index];
    return s.generation == i.generation ? s.pointer.get() : nullptr;
  }

  const T* get(const slotid& i) const noexcept {
    if (i.index >= _slots.size()) return nullptr;
    auto& s = _slots[i.index];
    return s.generation == i.generation ? s.pointer.get() : nullptr;
  }

  void erase(const slotid& i) {
    if (i.index >= _slots.size()) return;
    if (auto& s = _slots[i.index]; s.generation == i.generation) {
      s.pointer.reset();
      s.generation++;
      s.next_free = _free_head;
      _free_head = i.index;
      _erase(i.index);
    }
  }

  slotid push(std::unique_ptr<T, Del> p) {
    auto i = _push(std::move(p));
    _list.emplace_back(i.index, i.generation);
    return i;
  }

  void clear() {
    _slots.clear();
    _list.clear();
    _free_head = uint32_t(-1);
  }

  void set_order(const slotid& i, size_t pos) {
    if (i.index >= _slots.size()) return;
    if (auto& s = _slots[i.index]; s.generation != i.generation) return;
    if (auto it = std::ranges::find(_list, i); it != _list.end()) {
      size_t from = size_t(it - _list.begin());
      if (from != pos) _insert(from, pos);
    }
  }

  void bring_to_first(const slotid& i) {
    if (i.index >= _slots.size()) return;
    if (auto& s = _slots[i.index]; s.generation != i.generation) return;
    if (auto it = std::ranges::find(_list, i); it != _list.end()) {
      size_t from = size_t(it - _list.begin());
      if (from != 0) _insert(from, 0);
    }
  }

  void bring_to_last(const slotid& i) {
    if (i.index >= _slots.size()) return;
    if (auto& s = _slots[i.index]; s.generation != i.generation) return;
    if (auto it = std::ranges::find(_list, i); it != _list.end()) {
      size_t from = size_t(it - _list.begin());
      if (from != _list.size() - 1) _insert(from, _list.size() - 1);
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
    auto& operator++() {
      ++_pos;
      return *this;
    }
    auto operator++(int) {
      iterator temp = *this;
      ++_pos;
      return temp;
    }
    auto& operator--() {
      --_pos;
      return *this;
    }
    auto operator--(int) {
      iterator temp = *this;
      --_pos;
      return temp;
    }
    auto& operator+=(difference_type n) {
      _pos += n;
      return *this;
    }
    auto& operator-=(difference_type n) {
      _pos -= n;
      return *this;
    }
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
    auto& operator++() {
      ++_pos;
      return *this;
    }
    auto operator++(int) {
      const_iterator temp = *this;
      ++_pos;
      return temp;
    }
    auto& operator--() {
      --_pos;
      return *this;
    }
    auto operator--(int) {
      const_iterator temp = *this;
      --_pos;
      return temp;
    }
    auto& operator+=(difference_type n) {
      _pos += n;
      return *this;
    }
    auto& operator-=(difference_type n) {
      _pos -= n;
      return *this;
    }
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
  auto& operator[](size_t i) { return *get(_list[i]); }
  const auto& operator[](size_t i) const { return *get(_list[i]); }

  iterator find(const slotid& i) noexcept {
    if (i.index >= _slots.size()) return end();
    if (_slots[i.index].generation != i.generation) return end();
    for (size_t j = 0; j < _list.size(); ++j)
      if (_list[j] == i) return iterator(this, j);
    return end();
  }

  const_iterator find(const slotid& i) const noexcept {
    if (i.index >= _slots.size()) return end();
    if (_slots[i.index].generation != i.generation) return end();
    for (size_t j = 0; j < _list.size(); ++j)
      if (_list[j] == i) return const_iterator(this, j);
    return end();
  }
};
} // namespace yw
