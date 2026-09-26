#pragma once

#include "init_clockwise_atomic_workaround.frag.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char init_clockwise_atomic_workaround_frag[] = R"===(#ifdef EB
M1
#ifndef Q
z0(G2,m0);
#endif
z0(X2,i0);N1 k5(HB){A0(i0,E0(K0(i0).x,.0,.0,1.));
#ifndef Q
d4(K0(m0));
#else
d4(E0(.0));
#endif
}
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive