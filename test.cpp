#include "ywlib"
using namespace yw;

#include <iostream>

int main() {
  try {
    auto a1 = now();
    std::cout << a1 << std::endl;
    auto a2 = value(now);
    std::cout << a2 << std::endl;
    auto a3 = str1(now);
    std::cout << a3 << std::endl;
    auto a4 = str2(now);
    std::wcout << a4 << std::endl;
    auto a5 = string<uct1>(now);
    auto b5 = to_wcs(a5);
    std::wcout << b5 << std::endl;
    // static_assert(decltype(now)::j<str1> == 2);


    for (auto a = 0x81; a < 0xa0; ++a) {
      for (auto b = 0x40; b < 0xfd; ++b) {
        char c[] = {a, b, 0};
        auto d = to_wcs(c);
      }
    }


  } catch (const std::exception& E) {
    std::cout << E.what();
    return -1;
  }
  return 0;
}
