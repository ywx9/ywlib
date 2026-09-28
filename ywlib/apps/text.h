#pragma once
#include <apps/directx.h>

namespace yw {

///--------------------------------------------------------------------------///
/// MARK: font_config

struct font_config {
  string<wchar_t> name = L"";
  float size = 16.0f;
  DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL;
  DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL;
  DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL;
};

comptr<IDWriteTextFormat> create_text_format(const font_config& Config) {
  comptr<IDWriteTextFormat> result;
  const auto hr = dwrite::factory()->CreateTextFormat(
    Config.name.c_str(), nullptr, Config.weight, Config.style, Config.stretch, Config.size, L"", &result.get());
  if (FAILED(hr)) {
    error(errors::operation_failed, "CreateTextFormat failed").consume();
    return {};
  } else return result;
}

///--------------------------------------------------------------------------///
/// MARK: text

class text {
public:
  const_property<font_config, text> font;
  const_property<string<wchar_t>, text> string;
  const_property<comptr<IDWriteTextLayout>, text> dwrite_text_layout;
  const_property<float2, text> size;
};

} // namespace yw
