#include "ywlib.hpp"
using namespace yw;

int main() {
  std::vector<int> a(10, 3);
  std::vector<short> b(20, 6);
  float c[30]{};
  yw::array<int> d(yw::mv(a));

  static_assert(same_as<iter_reference_t<decltype(b)::iterator>, short&>);
  static_assert(same_as<iter_reference_t<decltype(b)>, short&>);
  static_assert(convertible_to<iter_reference_t<decltype(b)>, int>);
  static_assert(convertible_to<short, int>);
  static_assert(convertible_to<int, int>);
  static_assert(castable_to<int, int>);
  static_assert(input_range<decltype(b)>);
  static_assert(range<decltype(b)>);
  yw::array<int> e(b);
  yw::array<float> f(c);
  std::cout << f.capacity() << std::endl;
}
