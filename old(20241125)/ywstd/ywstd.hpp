#pragma once

#if defined(YWLIB) && YWLIB == 2 // for executable
import ywstd;
#else // for coding or module
#define _BUILD_STD_MODULE
#include "std.hpp"
#include "win.hpp"
#endif

////////////////////////////////////////////////////////////////////////////////
/// \note template variables need to be defined in header not in module
////////////////////////////////////////////////////////////////////////////////

namespace yw {}
