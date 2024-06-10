#include "ywlib"
using namespace yw;

#include <iostream>

int main() {
  list a{5, 12.0, 'C', true, nat(1)};
  auto sum = [](auto&&... args) { return (args + ...); };
  static_assert(invocable<decltype(sum), int, double, char, bool, nat>);
  // invoke(sum, get<0>(a), get<1>(a), get<2>(a), get<3>(a), get<4>(a));
  // _::_apply(sum, a);
  std::cout << "Sum of a: " << _::_apply(sum, 12, a, -5) << std::endl;
  // sum(a);
}
