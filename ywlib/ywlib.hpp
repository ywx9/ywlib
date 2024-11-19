#pragma once

#ifndef nat
#define nat size_t
#endif

#pragma warning(disable : 4459)

#if defined(YWLIB) && YWLIB == 2 // for executable
import ywlib;
#else // for coding or module
#define _BUILD_STD_MODULE
#include "ywlib-directx.hpp"
#include "ywlib-sjis.hpp"
#include "ywlib-std.hpp"
#include "ywlib-windows.hpp"
#include "ywlib-xv.hpp"
#endif

////////////////////////////////////////////////////////////////////////////////
/// \note template variables need to be defined in header not in module
////////////////////////////////////////////////////////////////////////////////

namespace yw {}
