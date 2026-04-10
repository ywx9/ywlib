#pragma once
#include "ywx/bitmap.h"
#include "ywx/text.h"

namespace yw::system {

class {
  HWND _hwnd{};
  bitmap _rendertarget;
  comptr<IDXGISwapChain1> _swapchain;
  text _text;


public:
  bool opened() const noexcept { return _opened; }
} ime;
}
