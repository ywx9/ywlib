#include "ywlib"
using namespace yw;

#include <iostream>

int main() {
  main::resize(800, 600);
  hsv background = hsv::red;
  auto tf = text_format<20, rgb::blue, L"Arial">{};
  std::cout << source{} << std::endl;
  std::cout << std::format("{}", tf.geometry(L"Hello, World"));
  std::cout << source{} << std::endl;
  while (main::update) {
    background.h += fat4(main::spf) / 2.f;
    while (background.h > 1.0f) background.h -= 1.0f;
    // std::cout << std::format("{}", background) << std::endl;
    auto _rgb = rgb(background);
    // std::cout << std::format("{}", _rgb) << std::endl;
    main::begin_draw(_rgb);
    tf.print(L"Hello, World", 100, 100);
    main::end_draw();
  }
}
