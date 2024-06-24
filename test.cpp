#include "ywlib"
using namespace yw;

#include <iostream>

int main() {
  main::resize(800, 600);
  hsv background = hsv::red;
  auto tf = text_format<20, rgb::blue, L"Arial">{};
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
  auto text = std::format(L"{}", i);
  auto center = tf.geometry(text, {400, 300}, alignment::center);
  auto geo = geometry();
  geo.add_line({0, 0}, {800, 600});
  auto sb = solid_brush{rgb::green};
  while (main::update) {
    if (key::space.changed() && key::space.get()) {
      text = std::format(L"{}", ++i);
      center = tf.geometry(text, {400, 300}, alignment::center);
    }
    if (key::mbutton.changed()) {
      if (key::mbutton.get()) std::cout << "LButton Down" << std::endl;
      else std::cout << "LButton Up" << std::endl;
    }
    background.h += fat4(main::spf) / 2.f;
    while (background.h > 1.0f) background.h -= 1.0f;
    // std::cout << std::format("{}", background) << std::endl;
    auto _rgb = rgb(background);
    // std::cout << std::format("{}", _rgb) << std::endl;
    main::begin_draw(_rgb);
    tf.draw(L"Hello, World", lt);
    tf.draw(L"Hello, World", top);
    tf.draw(L"Hello, World", rt);
    tf.draw(L"Hello, World", left);
    tf.draw(text, center);
    tf.draw(L"Hello, World", right);
    tf.draw(L"Hello, World", lb);
    tf.draw(L"Hello, World", bottom);
    tf.draw(L"Hello, World", rb);
    geo.draw(sb, 2.f);
    main::end_draw();
  }
}
