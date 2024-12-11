module;
#ifndef YWSTD_IMPORT
#define YWSTD_IMPORT false
#endif
#ifndef YWLIB_COMPILE
#define YWLIB_COMPILE false
#endif
export module ywlib;
#include "ywlib.hpp"
static_assert(YWSTD_IMPORT, "This source file needs `module ywstd` to be imported");
static_assert(YWLIB_COMPILE, "This source file is used for compiling `module ywlib`");
