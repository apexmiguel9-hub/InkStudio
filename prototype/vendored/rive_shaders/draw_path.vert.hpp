#pragma once

#include "draw_path.vert.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char draw_path_vert[] = R"===(#undef B2
#ifdef GB
#define B2 f
#else
#define B2 D
#endif
#ifdef CB
h1(g0)
#if defined(DB)||defined(FB)
K(0,R3,JB);
#else
K(0,f,VB);K(1,f,WB);
#endif
i1
#endif
q2 I0 W(0,f,X1);
#ifdef FB
I0 W(1,c,F2);
#elif!defined(BB)
#ifdef DB
MB W(1,d,j1);
#else
I0 W(2,B2,O);
#endif
MB W(3,d,D0);
#endif
#ifdef I
#ifdef FB
MB W(4,d,O3);
#else
MB W(4,D,Y1);
#endif
#endif
#if defined(AB)&&!defined(BB)
I0 W(5,f,O0);
#endif
#ifdef T
MB W(6,d,g1);
#endif
#ifdef QB
V2 W(7,N0,k3);W(8,c,v4);
#endif
#ifdef IB
I0 W(9,S,C2);
#endif
i2
#ifdef CB
#ifdef KD
Ld(Dh) Md(float,pi) Nd(qi)
#endif
B1(EC,g0,F,A,r){
#if defined(DB)||defined(FB)
L(A,F,JB,S);
#else
L(A,F,VB,f);L(A,F,WB,f);
#endif
V(X1,f);
#if defined(IB)
V(C2,S);
#endif
#ifdef FB
V(F2,c);
#elif!defined(BB)
#ifdef DB
V(j1,d);
#else
V(O,B2);
#endif
V(D0,d);
#endif
#ifdef I
#ifdef FB
V(O3,d);
#else
V(Y1,D);
#endif
#endif
#if defined(AB)&&!defined(BB)
V(O0,f);
#endif
#ifdef T
V(g1,d);
#endif
#ifdef QB
V(k3,N0);V(v4,c);
#endif
bool ve=false;uint o0;c l0;
#ifdef BB
N h9;
#endif
#ifdef FB
l0=Jb(JB,o0,
#ifdef BB
h9,
#endif
F2 A3);
#elif defined(DB)
l0=Kb(JB,o0
#ifdef BB
,h9
#else
,j1
#endif
A3);
#else
f P;ve=!r9(VB,WB,r,o0,l0
#ifndef BB
,P
#else
,h9
#endif
A3);
#ifndef BB
#ifdef GB
O=P;
#else
O.xy=R7(P.xy);
#endif
#endif
#endif
N0 r1=R5(CD,o0);
#if!defined(FB)&&!defined(BB)
D0=r8(o0,j.c6);if((r1.x&K9)!=0u) D0=-D0;
#endif
uint X3=r1.x&0xfu;
#ifdef I
if(I){uint ri=(X3==Z7?r1.y:r1.x)>>16;d m1=r8(ri,j.c6);if(X3==Z7) m1=-m1;
#ifdef FB
O3=m1;
#else
Y1.x=m1;
#endif
}
#endif
#ifdef T
if(T){g1=float((r1.x>>4)&0xfu);}
#endif
c v0=l0;
#ifdef ME
if(j.Mb!=0u){v0.y=float(j.Nb)-v0.y;}
#endif
#ifdef AB
if(AB){e0 e4=L1(L0(PB,o0*E3+2u));f J4=L0(PB,o0*E3+3u);
#ifndef BB
O0=T7(e4,J4.xy,v0);
#else
Mc(e4,J4.xy,v0 A5);
#endif
}
#endif
if(X3==Vb){X1=f(unpackUnorm4x8(r1.y));}
#if defined(I)&&!defined(FB)
else if(I&&X3==Z7){d K5=r8(r1.x>>16,j.c6);Y1.y=K5;}
#endif
else{e0 si=L1(L0(PB,o0*E3));f we=L0(PB,o0*E3+1u);X1=Ob(v0,si,we.xy,float(X3),we.zw,uintBitsToFloat(r1.y));X1.w=-X1.w;}
#ifdef KD
if(KD){X1*=qi.pi;}
#endif
#if defined(IB)
if(IB&&(r1.x&ig)!=0u){e0 ti=L1(L0(PB,o0*E3+4u));f xe=L0(PB,o0*E3+5u);c k4=P0(ti,v0)+xe.xy;C2=S(k4.x,k4.y,1.+xe.z);}else{C2=S(0.0,0.0,0.0);}
#endif
f X;if(!ve){X=Q3(l0);
#ifdef RC
X.y=-X.y;
#endif
#ifdef BB
X.z=ma(h9,0xffu);
#elif defined(QB)
R U4=L0(OB,o0*4u+3u);k3=U4.xy;v4=l0+uintBitsToFloat(U4.zw);
#endif
}else{X=f(j.W2,j.W2,j.W2,j.W2);}c0(X1);
#if defined(IB)
c0(C2);
#endif
#ifdef FB
c0(F2);
#elif!defined(BB)
#ifdef DB
c0(j1);
#else
c0(O);
#endif
c0(D0);
#endif
#ifdef I
#ifdef FB
c0(O3);
#else
c0(Y1);
#endif
#endif
#if defined(AB)&&!defined(BB)
c0(O0);
#endif
#ifdef T
c0(g1);
#endif
#ifdef QB
c0(k3);c0(v4);
#endif
C1(X);}
#endif
#ifdef EB
U3 V3 e i L7(
#ifdef IB
S sb,
#endif
#ifdef T
N p3,
#endif
f V4 L6){
#ifdef T
bool d5=T&&p3!=B4;
#else
const bool d5=false;
#endif
i k;if(V4.w>=.0){k=g5(V4);}else{V4.w=-V4.w;d N9=S3(fract(V4.w)*(256./255.));V4.w=floor(V4.w)*j.Zb+j.ac;c T9=fc(V4);k=j2(DD,M9,T9,.0);if(!d5){k.xyz*=k.w;k.w*=N9;}}
#if defined(IB)
if(IB&&sb.z>0.0){d ui=sb.z-1.;i k2=U6(GC,W5,sb.xy,ui);if(d5) k2=E0(F6(k2),k2.w);k*=k2;}
#endif
return k;}
#if!defined(DB)&&!defined(FB)
e d ye(B2 P L3){
#ifdef GB
if(GB&&bc(P)) return C4(P e1);else
#endif
return min(P.x,P.y);}e d ze(B2 P L3){
#if defined(GB)
if(GB&&cc(P)) return d8(P e1);else
#endif
return P.x;}e d tb(B2 P L3){if(V5(P)) return ye(P e1);else return ze(P e1);}e d vi(d W4,B2 P L3){if(V5(P)){d y0=ye(P e1);return max(y0,W4);}else{d y0=ze(P e1);return W4+y0;}}
#endif
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive