#include "ywlib"
using namespace yw;

#include <iostream>

int main() {
  auto u16 = u"この文章はUTF16で書かれました❤";
  auto u8 = u8"この文章はUTF8で書かれました♪";
  ok(reinterpret_cast<const cat2*>(u16));
  auto s = to_wcs(u8);
  ok(s);
}
