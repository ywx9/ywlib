#include "ywlib"
using namespace yw;

#include <iostream>

int main() {
  try {
    main::resize(800, 600);
    color::hsv background = color::red;
    auto tf = text_format(L"Arial", 20);
    std::cout << source{} << std::endl;
    // std::cout << std::format("{}", tf.geometry(L"Hello, World"));
    std::cout << source{} << std::endl;
    auto lt = tf.geometry(L"Hello, World", {0, 0}, alignment::lefttop);
    auto top = tf.geometry(L"Hello, World", {400, 0}, alignment::top);
    auto rt = tf.geometry(L"Hello, World", {800, 0}, alignment::righttop);
    auto left = tf.geometry(L"Hello, World", {0, 300}, alignment::left);
    auto right = tf.geometry(L"Hello, World", {800, 300}, alignment::right);
    auto lb = tf.geometry(L"Hello, World", {0, 600}, alignment::leftbottom);
    auto bottom = tf.geometry(L"Hello, World", {400, 600}, alignment::bottom);
    auto rb = tf.geometry(L"Hello, World", {800, 600}, alignment::rightbottom);
    nat i = 0;
    std::cout << source{} << std::endl;
    auto text = std::format(L"{}", i);
    std::cout << source{} << std::endl;
    auto center = tf.geometry(text, {400, 300}, alignment::center);
    std::cout << source{} << std::endl;
    auto geo = geometry();
    std::cout << source{} << std::endl;
    // geo.add_line({0, 0}, {800, 600});
    std::cout << source{} << std::endl;
    geo.add_rectangle({100, 100, 700, 500});
    std::cout << source{} << std::endl;
    auto sb = brush{color::green};
    std::cout << source{} << std::endl;
    while (main::update) {
      if (key::space.changed && key::space()) {
        text = std::format(L"{}", ++i);
        center = tf.geometry(text, {400, 300}, alignment::center);
      }
      std::cout << source{} << std::endl;
      if (key::mbutton.changed) {
        if (key::mbutton()) std::cout << "LButton Down" << std::endl;
        else std::cout << "LButton Up" << std::endl;
      }
      std::cout << source{} << std::endl;
      background.h += fat4(main::spf) / 2.f;
      while (background.h > 1.0f) background.h -= 1.0f;
      // std::cout << std::format("{}", background) << std::endl;
      color _rgb = background;
      // std::cout << std::format("{}", _rgb) << std::endl;
      std::cout << source{} << std::endl;
      main::begin_draw(_rgb);
      std::cout << source{} << std::endl;
      tf.draw(L"Hello, World", lt);
      std::cout << source{} << std::endl;
      tf.draw(L"Hello, World", top);
      tf.draw(L"Hello, World", rt);
      tf.draw(L"Hello, World", left);
      tf.draw(text, center);
      std::cout << source{} << std::endl;
      tf.draw(L"Hello, World", right);
      tf.draw(L"Hello, World", lb);
      tf.draw(L"Hello, World", bottom);
      tf.draw(L"Hello, World", rb);
      std::cout << source{} << std::endl;
      geo.draw(sb, 2.f);
      std::cout << source{} << std::endl;
      main::end_draw();
    }
  } catch (const std::exception& e) {
    yw::log << e.what();
    std::cout << e.what() << std::endl;
  }
}
