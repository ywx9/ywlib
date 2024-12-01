#pragma once
#if defined(YWLIB) && YWLIB == 2 // for executable
import ywstd;
#else // for coding or compiling module
#define _BUILD_STD_MODULE
#include "intrin.hpp"
#include "sjis.hpp"
#include "std.hpp"
#include "win.hpp"
#endif

#ifndef nat
#define nat size_t
#endif
