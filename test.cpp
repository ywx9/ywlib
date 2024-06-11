#include "ywlib"
using namespace yw;

#include <iostream>

int main() {
  int array[] = {1, 2, 3, 4, 5};
  // static_assert(same_extent<list<int, int>>);
  auto v = _::_vapply([](auto a, auto b) { return a + b; }, array, 5);
  [&]<nat... Is>(sequence<Is...>) {
    ((std::cout << get<Is>(v) << std::endl), ...);
  }(make_indices_for<decltype(v)>{});
  std::cout << apply(max, vapply([](auto a) { return a * a; }, array)) << std::endl;
}
