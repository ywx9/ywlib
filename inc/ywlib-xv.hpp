/// \file ywlib-xv.hpp

#pragma once

#include "ywlib-core.hpp"

export namespace yw {

using xvector = mm::m128;

/// loads four values from the memory
inline xvector xvload(const fat4* p) noexcept { return mm::loadu_ps<mm::m128>(p); }

/// loads four values from the memory in reverse order
/// \note `p` must be aligned to 16 bytes
inline xvector xvloadr(const fat4* p) noexcept { return mm::loadr_ps(p); }

/// sets values to the vector
inline xvector xvset(const fat4 a, const fat4 b, const fat4 c, const fat4 d) noexcept  { return mm::set_ps(d, c, b, a); }



}
