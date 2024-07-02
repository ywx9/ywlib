#include "ywlib"
using namespace yw;

#include <iostream>

int main() {
  try {
    std::cout << str1(now) << std::endl;
    std::wcout << str2(now) << std::endl;
    std::cout << to_mbs(string_view<uct1>(now)) << std::endl;
    std::wcout << to_wcs(string_view<uct2>(now)) << std::endl;
    auto s = to_wcs(string_view<uct4>(now));
    std::wcout << s << std::endl;
    std::wcout << s.size() << std::endl;


    yw::log.debug(string_view<uct1>(u8"debug test"));
    std::cout << yw::log;
  } catch (const std::exception& E) {
    std::cout << E.what();
    return -1;
  }
  return 0;
}
