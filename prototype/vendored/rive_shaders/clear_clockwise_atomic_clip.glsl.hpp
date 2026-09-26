#pragma once

#include "clear_clockwise_atomic_clip.glsl.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char clear_clockwise_atomic_clip[] = R"===(#ifdef CB
h1(g0) K(0,R3,JB);i1 B1(EC,g0,F,A,r){L(A,F,JB,R3);f X=Q3(JB.xy);C1(X);}
#endif
#ifdef EB
M1
#ifndef Q
z0(G2,m0);
#endif
z0(X2,i0);N1 k5(HB){A0(i0,E0(.0,.0,.0,1.));d4(E0(.0));}
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive