module;
#include "yw.hpp"
export module yw.x0 :array;
import <vector>;
export namespace yw {
  export template<typename T, natt N> using agent_array = std::array<T, N>;
  export template<typename T> using varray = std::vector<T>;

  export template<typename T> class view_array {
    static_assert(!std::is_array_v<T>&& std::is_trivially_destructible_v<T>&& std::is_standard_layout_v<T>);
    const T* _data;
    natt _size;
  public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using const_iterator = const T*;
    using iterator = const_iterator;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator = const_reverse_iterator;
    using size_type = natt;
    using difference_type = intt;

    static constexpr auto npos{static_cast<natt>(-1)};

    constexpr view_array(void)noexcept : _data(nullptr), _size(0) {}
    constexpr view_array(const view_array&)noexcept = default;
    constexpr view_array& operator=(const view_array&)noexcept = default;
    template<typename U, typename N> constexpr view_array(U(&arr)[N])
  };
}
