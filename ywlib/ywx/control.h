#pragma once
#include "ywx/core.h"

namespace yw {

namespace ui {

enum class alignment { center, left, right };
enum class valignment { middle, top, bottom };

class control {
public:
  class slot;

protected:
  slotset<slot>::slotid _id;
  std::optional<float> _width;
  std::optional<float> _height;
  std::optional<float2> _margin;
  std::optional<alignment> _alignment;
  std::optional<valignment> _valignment;

  control(slotset<slot>::slotid Id) : _id(Id) {}

public:
  control() = default;
};
} // namespace ui

//////////////////////////////////////// MARK: system::controls

namespace system {
inline slotset<ui::control::slot> controls;
}

//////////////////////////////////////// MARK: control::slot

class ui::control::slot {
public:
  slotset<slot>::slotid id;
  std::optional<float> width;
  std::optional<float> height;
  std::optional<float2> margin;
  std::optional<alignment> alignment;
  std::optional<valignment> valignment;
};
} // namespace yw
