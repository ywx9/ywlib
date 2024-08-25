// for test

#ifndef YWLIB
#include "inc/ywlib.hpp"
#else
import ywlib;
#endif
using namespace yw;


int main() {

  auto a = list{1, 2, 3, 4, 5};

  auto b = list{1.1, 2.2, 3.3, 4.4, 5.5};

  auto c = vapply([](auto x, auto y) { return x + y; }, a, b);

  vapply([](auto x) { std::cout << x << std::endl; }, c);

  vapply([](auto x) { std::cout << x << std::endl; }, projector(c, [](auto x) { return x * 2; }, sequence<3, 2, 1, 0>{}));

  std::print("{}\n", source{});
}
