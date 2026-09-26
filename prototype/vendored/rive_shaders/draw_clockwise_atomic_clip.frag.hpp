#pragma once

#include "draw_clockwise_atomic_clip.frag.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char draw_clockwise_atomic_clip_frag[] = R"===(#ifdef EB
M1
#ifndef Q
z0(G2,m0);
#endif
Td(X2,i0);N1
#ifdef JD
U3 Sa(sa,Ae,S0);V3
#endif
#ifdef Q
#define k5 v2
#define d4(x5) F1=x5;r3
#else
#define k5 P1
#define d4(x5) A0(m0,x5);d2;
#endif
k5(HB){
#ifdef DB
q(j1,d);d y0=j1;
#else
q(O,B2);d y0=O.x;
#endif
#ifdef JD
if(JD){q(k3,N0);q(v4,c);uint M7=k3.y;uint Z1=k3.x+L8(N0(floor(v4)),M7);uint o1=Rd(S0,Z1);d ub;if(y0>=1.&&(o1<j.f2||o1>=(j.f2|n5))){ub=.0;}else{d Ce=y0;d i9=y0;if(o1<j.f2){uint N7=j.f2|(n5+w7(abs(y0)));uint l3=C7(S0,Z1,N7);if(l3<=j.f2){i9=.0;}else if(l3<N7){i9=Oa(l3);}}if(i9>.0){uint vb=Ta(S0,Z1,w7(abs(i9)));Ce=Oa(vb)+y0;}ub=1.-Ce;}A0(i0,E0(ub));d4(E0(1.))}else
#endif
{A0(i0,E0(y0));d4(E0(.0))}}
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive