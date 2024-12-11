module;
#ifndef YWSTD_COMPILE
#define YWSTD_COMPILE false
#endif
export module ywstd;
#pragma warning(push)
#pragma warning(disable: 5244)
#include "ywstd.hpp"
#pragma warning(pop)
static_assert(YWSTD_COMPILE, "This source file is used for compiling `module ywstd`");
