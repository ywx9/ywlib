#pragma once
#if defined(YWLIB) && YWLIB == 2 // for executable
import ywstd;
#else // for coding or compiling module
#define _BUILD_STD_MODULE
#include "intrin.hpp"
#include "std.hpp"
#endif
