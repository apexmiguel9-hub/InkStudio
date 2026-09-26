#pragma once

#include "stencil_draw.glsl.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char stencil_draw[] = R"===(#ifdef CB
h1(g0) K(0,R3,JB);i1 Y3 Z3 F4 G4 B1(DG,g0,F,A,r){L(A,F,JB,R3);f X=Q3(JB.xy);uint Xh=floatBitsToUint(JB.z)&0xffffu;X.z=ma(Xh,0xffu);C1(X);}
#endif
#ifdef EB
I3 J3 f3(i,QE){M2(E0(.0));}
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive