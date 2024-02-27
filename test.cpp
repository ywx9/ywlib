#define ywlib_debug
#include "ywlib/control.h"
using namespace yw;

namespace {

constexpr auto app_name = "test";

constexpr natt app_width = 960, app_height = 540;

auto key_configs = [] { key::escape.down = []() { yes(L"終了しますか？") ? main::terminate() : void(); };
                        return none{}; }();

button ui_button_cmt{};
button ui_button_cad{};
label ui_label_cmt{};
label ui_label_cad{};
radiobutton ui_radiobutton_flip{};
checkbox ui_checkbox_reverse{};
checkbox ui_checkbox_transparent{};
button ui_button_align{};
label ui_label_align{}; // 計算時間
label ui_label_rotation{};
label ui_label_rot_x{};
label ui_label_rot_y{};
label ui_label_rot_z{};
valuebox ui_valuebox_rot[3];
label ui_label_offset{};
label ui_label_off_x{};
label ui_label_off_y{};
label ui_label_off_z{};
valuebox ui_valuebox_off[3];
label ui_label_min_whole{};
label ui_label_min_over{};
label ui_label_max_over{};
button ui_button_result{};
textbox ui_textbox_result[7]; // Group 2

}

void ywmain() {
  main::resize(400, 400);
  auto rb = radiobutton(rect{10, 10, 390, 390}, L"Radio Button", list{L"First", L"Second", L"Last"}, WS_BORDER);
  rb.show();
  rb.input = [](const radiobutton& This) { std::cout << This.state << std::endl; };
  key::escape.down = [] { main::terminate(); };
  while (main::update) {
    main::begin_draw(color::white);
    main::end_draw();
  }
}
