// for test

#ifndef YWLIB
#include "inc/ywlib.hpp"
#else
import ywlib;
#endif
using namespace yw;

int A[1000];
void test() {
#pragma loop(hint_parallel(0))
  for (int i=0; i<1000; ++i) {
    A[i] = A[i] + 1;
  }

  // for (int i=1000; i<2000; ++i) {
  //   A[i] = A[i] + 1;
  // }
}

int main() {

auto a = vector{1, 2, 3, 4};
auto b = vector{5, 6, 7, 8};

auto c = a + b;
std::print("c = a + b = {}\n", c);

c = vector{a.x, a.y, b.x, b.y};
std::print("{}\n", c);

}
