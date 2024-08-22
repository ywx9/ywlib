// for test

#ifndef YWLIB
#include "inc/ywlib.hpp"
#else
import ywlib;
#endif
using namespace yw;


int main() {

  std::print("{}\n", none{});
  std::wcout << std::format(L"{}\n", none{});
}
