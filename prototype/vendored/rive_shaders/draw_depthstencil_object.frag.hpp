#pragma once

#include "draw_depthstencil_object.frag.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char draw_depthstencil_object_frag[] = R"===(#ifdef EB
#ifdef KB
I3 e3(h5,a4,GC);
#ifdef T
p5(XD);
#endif
J3 i5 c4(W5) j5
#endif
f3(i,HB){
#ifdef KB
q(J5,c);q(K1,i);
#ifdef T
q(D1,N);
#endif
#else
q(X1,f);
#ifdef IB
q(C2,S);
#endif
#ifdef FB
q(F2,c);
#endif
#ifdef T
q(g1,d);
#endif
#endif
#ifdef KB
i k=A7(GC,W5,J5,j.Ed)*K1;
#else
d o=
#ifdef FB
clamp(j2(ED,R9,F2,.0).x,J0(.0),J0(1.));
#else
1.;
#endif
i k=L7(
#ifdef IB
C2,
#endif
#ifdef T
g3(g1),
#endif
X1 Y2);
#endif
#if defined(T)&&!defined(Q)
#ifdef KB
k.xyz=F6(k);N p3=D1;
#else
N p3=g3(g1);
#endif
i O1=x6(XD);k.xyz=Y4(k.xyz,O1,p3)*k.w;
#endif
#ifndef KB
k*=o;
#endif
#ifdef BC
if(BC){k=q3(k);}
#endif
k.xyz=K2(k.xyz,k.w,d0.xy,j.F3,j.G3);M2(k);}
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive