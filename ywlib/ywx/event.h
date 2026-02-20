#pragma once
#include "ywx/core.h"

namespace yw {

struct hover_event {
  uint64_t control_kind{};
  float2 position{};
  bool hovered{};
};
}
