#pragma once
#include <base/array.h>
#include <core/string.h>

namespace yw {

template<typename T> requires constructible<T> class dict {
  static constexpr size_t _block_size = 32;
  static constexpr size_t _split_size = _block_size / 2;

  struct _entry {
    string<char> key;
    T value;
  };
  struct _block {
    array<_entry, _block_size> entries;
    size_t count = 0;
  };

  array<_block*> _blocks;

  struct _location {
    size_t block;
    size_t entry;
    bool found;
  };

  static constexpr _block* _new_block() noexcept {
    auto block = yw::allocate<_block>();
    std::construct_at(block);
    return block;
  }

  static constexpr void _delete_block(_block* block) noexcept {
    std::destroy_at(block);
    yw::deallocate(block);
  }

  constexpr _location _locate(string_view<char> key) const noexcept {
    size_t first = 0, last = _blocks.size();
    while (first < last) {
      const auto middle = first + (last - first) / 2;
      const auto& block = *_blocks[middle];
      if (string_view<char>(block.entries[block.count - 1].key) < key) first = middle + 1;
      else last = middle;
    }
    if (first == _blocks.size()) return {first, 0, false};

    const auto& block = *_blocks[first];
    size_t entry_first = 0, entry_last = block.count;
    while (entry_first < entry_last) {
      const auto middle = entry_first + (entry_last - entry_first) / 2;
      if (string_view<char>(block.entries[middle].key) < key) entry_first = middle + 1;
      else entry_last = middle;
    }
    const bool found = entry_first < block.count && string_view<char>(block.entries[entry_first].key) == key;
    return {first, entry_first, found};
  }

  constexpr void _insert_block(size_t index, _block* block) noexcept {
    _blocks.push_back(block);
    for (auto i = _blocks.size() - 1; i > index; --i) _blocks[i] = move(_blocks[i - 1]);
    _blocks[index] = block;
  }

  template<typename SetValue> constexpr _entry& _insert(string_view<char> key, SetValue&& set_value) {
    auto location = _locate(key);
    if (location.found) return _blocks[location.block]->entries[location.entry];

    if (location.block == _blocks.size()) {
      auto block = _new_block();
      _blocks.push_back(block);
      location = {_blocks.size() - 1, 0, false};
    }

    auto* block = _blocks[location.block];
    if (block->count < _block_size) {
      for (auto i = block->count; i > location.entry; --i) block->entries[i] = move(block->entries[i - 1]);
      auto& entry = block->entries[location.entry];
      entry.key = key;
      set_value(entry.value);
      ++block->count;
      size = size() + 1;
      return entry;
    }

    auto* right = _new_block();
    _insert_block(location.block + 1, right);
    if (location.entry <= _split_size) {
      for (size_t i = _split_size; i < _block_size; ++i) right->entries[i - _split_size] = move(block->entries[i]);
      for (auto i = _split_size; i > location.entry; --i) block->entries[i] = move(block->entries[i - 1]);
      auto& entry = block->entries[location.entry];
      entry.key = key;
      set_value(entry.value);
      block->count = _split_size + 1;
      right->count = _split_size;
      size = size() + 1;
      return entry;
    }

    const auto right_entry = location.entry - _split_size;
    for (size_t i = _split_size; i < location.entry; ++i) right->entries[i - _split_size] = move(block->entries[i]);
    for (size_t i = location.entry; i < _block_size; ++i) right->entries[i - _split_size + 1] = move(block->entries[i]);
    auto& entry = right->entries[right_entry];
    entry.key = key;
    set_value(entry.value);
    block->count = _split_size;
    right->count = _split_size + 1;
    size = size() + 1;
    return entry;
  }

public:
  const_property<size_t, dict> size = 0;

  constexpr ~dict() noexcept { clear(); }
  constexpr dict() noexcept = default;

  constexpr dict(const dict& other) requires constructible<T, const T&> && assignable<T&, const T&> {
    for (auto* source : other._blocks) {
      auto* block = _new_block();
      for (size_t i = 0; i < source->count; ++i) block->entries[i] = source->entries[i];
      block->count = source->count;
      _blocks.push_back(block);
    }
    size = other.size();
  }

  constexpr dict& operator=(const dict& other) requires constructible<T, const T&> && assignable<T&, const T&> {
    if (this != &other) {
      dict copy(other);
      swap(copy);
    }
    return *this;
  }

  constexpr dict(dict&& other) noexcept : _blocks(move(other._blocks)), size(other.size()) { other.size = 0; }

  constexpr dict& operator=(dict&& other) noexcept {
    if (this != &other) {
      clear();
      _blocks = move(other._blocks);
      size = other.size();
      other.size = 0;
    }
    return *this;
  }

  constexpr bool empty() const noexcept { return size() == 0; }

  constexpr T* find(string_view<char> key) noexcept {
    const auto location = _locate(key);
    return location.found ? &(_blocks[location.block]->entries[location.entry].value) : nullptr;
  }
  constexpr const T* find(string_view<char> key) const noexcept {
    const auto location = _locate(key);
    return location.found ? &(_blocks[location.block]->entries[location.entry].value) : nullptr;
  }
  constexpr bool contains(string_view<char> key) const noexcept { return find(key) != nullptr; }

  constexpr T& operator[](string_view<char> key) {
    return _insert(key, [](T&) noexcept {}).value;
  }

  template<typename U> requires assignable<T&, U&&>
  constexpr T& insert(string_view<char> key, U&& value) {
    if (auto* target = find(key)) {
      *target = static_cast<U&&>(value);
      return *target;
    }
    return _insert(key, [&value](T& target) { target = static_cast<U&&>(value); }).value;
  }

  constexpr void clear() noexcept {
    for (auto* block : _blocks) _delete_block(block);
    _blocks.clear();
    size = 0;
  }

  constexpr void swap(dict& other) noexcept {
    _blocks.swap(other._blocks);
    std::ranges::swap(size.ref(), other.size.ref());
  }

};
} // namespace yw
