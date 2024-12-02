#pragma once
#if defined(YWLIB) && YWLIB == 2 // for executable
import ywlib;
#else // for coding or compiling module
#define _BUILD_STD_MODULE
#include "sjis.hpp"
#include "std.hpp"
#include "win.hpp"
#include "xv.hpp"
#endif
