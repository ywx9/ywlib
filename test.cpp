#define ywlib_debug
#include "ywlib/main.h"
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


}
