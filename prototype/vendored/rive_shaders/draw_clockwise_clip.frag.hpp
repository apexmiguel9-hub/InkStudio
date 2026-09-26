#pragma once

#include "draw_clockwise_clip.frag.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char draw_clockwise_clip_frag[] = R"===(#ifdef EB
M1
#ifndef Q
z0(G2,m0);
#endif
k1(X2,i0);
#ifndef Q
Ya(f6,p4);
#endif
k1(J6,S0);N1 P1(HB){q(Y1,D);d m1=-Y1.x;
#ifdef DB
q(j1,d);d y0=j1;
#else
q(O,B2);d y0=O.x;
#endif
z2;D Q0;d M5,z3;
#if defined(DB)&&defined(CC)
if(CC){z3=y0;}else
#endif
{Q0=unpackHalf2x16(a1(i0));M5=Q0.y;d W4=M5==m1?Q0.x:J0(.0);z3=W4+y0;}
#ifdef YC
d K5=Y1.y;if(YC&&K5!=.0){d w4=.0;
#if defined(DB)&&defined(CC)
if(CC){Q0=unpackHalf2x16(a1(i0));M5=Q0.y;}
#endif
if(M5!=m1){w4=M5==K5?Q0.x:.0;d1(S0,packHalf2x16(D2(w4,hg)));}else{w4=unpackHalf2x16(a1(S0)).x;h2(S0);}z3=min(z3,w4);}else
#endif
{h2(S0);}d1(i0,packHalf2x16(D2(z3,m1)));
#ifndef Q
y2(m0);
#endif
A2;d2;}
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive