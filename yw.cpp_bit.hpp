#pragma once
#include "yw.hpp"
#ifdef VSCODE
#include <bit>
#else
import <bit>;
#endif

namespace yw { namespace cpp {

using std::bit_cast;
using std::has_single_bit;
using std::bit_ceil;
using std::bit_floor;
using std::bit_width;

using std::rotl;
using std::rotr;

using std::countl_zero;
using std::countl_one;
using std::countr_zero;
using std::countr_one;
using std::popcount;

using std::endian;

}}