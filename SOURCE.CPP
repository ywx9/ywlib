// for test

#ifndef YWLIB
#include "inc/ywlib.hpp"
#else
import ywlib;
#endif
using namespace yw;

int main() {

  xvector a(1, 2, 3, 4);
  xvector b(-1, -2, -3, -4);

  auto c = a + b;

  fat4 r[4];
  store(r, c);

  std::print("{}, {}, {}, {}\n", r[0], r[1], r[2], r[3]);
}
