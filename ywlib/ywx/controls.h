#pragma once
#include <ywx/text_format.h>
#include "ywx/window.h"

namespace yw {

//////////////////////////////////////// MARK: label

class label : public control {
public:
  text_format format{};

};
}
