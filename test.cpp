#include "ywlib"
using namespace yw;

#include <iostream>

int main() {
  main::resize(800, 600);
  hsv background = hsv::red;
  while (main::update) {
    background.h += fat4(main::spf) / 10.f;
    while (background.h > 1.0f) background.h -= 1.0f;
    std::cout << std::format("{}", background) << std::endl;
    auto _rgb = rgb(background);
    std::cout << std::format("{}", _rgb) << std::endl;
    main::begin_draw(_rgb);
    main::end_draw();
  }
}
