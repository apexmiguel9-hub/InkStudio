#pragma once

#include "draw_clockwise_atomic_path.frag.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char draw_clockwise_atomic_path_frag[] = R"===(#ifdef EB
M1
#ifndef Q
z0(G2,m0);
#endif
z0(X2,i0);
#ifndef Q
Xa(f6,D6);
#endif
N1 U3 Sa(sa,Ae,S0);V3 e d xi(float j3,d y0,uint Z1,c1(uint) o1,c1(d) P3){
#ifdef Q
if(min(j3,y0)>=1.){return 1.;}
#endif
d E;uint De=w7(abs(y0));o1=C7(S0,Z1,j.f2|De);if(o1<j.f2){E=y0;
#ifndef Q
P3=y0;
#endif
}else{
#ifndef Q
if((o1&i7)!=0u){o1=C7(S0,Z1,j.f2|i7|De);}
#endif
d e2=Z5(o1&wa)*ua;d H1=max(e2,y0);E=K8(e2,H1,j3);
#ifndef Q
P3=H1;
#endif
}return E;}e d yi(float j3,d X4,uint Z1,c1(uint) o1,c1(d) P3){d E=.0;uint wb=w7(abs(X4));o1=Rd(S0,Z1);
#ifdef Q
if(min(j3,X4)>=1.&&(o1<j.f2||o1>=(j.f2|n5))){return 1.;}
#endif
if(o1<j.f2){uint Ee=j.f2|(n5+wb);uint l3=C7(S0,Z1,Ee);
#ifndef Q
o1=l3;
#endif
if(l3<=j.f2){E=X4;
#ifdef DB
E=min(E,1.);
#endif
#ifndef Q
P3=E;
#endif
X4=.0;}else if(l3<Ee){uint Fe=(l3&wa)-n5;d e2=Z5(Fe)*ua;d H1=X4;
#ifdef DB
H1=min(H1,1.);
#endif
#ifndef Q
P3=H1;
#endif
E=K8(e2,H1,j3);wb=Fe;X4=e2;}}if(X4>.0){uint vb=Ta(S0,Z1,wb);d e2=Oa(vb);d H1=e2+X4;e2=clamp(e2,.0,1.);H1=clamp(H1,.0,1.);
#ifndef Q
P3=H1;
#endif
E+=(1.-E*j3)*K8(e2,H1,j3);}return E;}k5(HB){q(X1,f);
#ifdef IB
q(C2,S);
#endif
#ifdef DB
V(j1,d);
#else
V(O,B2);
#endif
q(D0,d);
#ifdef I
q(Y1,D);
#endif
#ifdef AB
q(O0,f);
#endif
#ifdef T
q(g1,d);
#endif
q(k3,N0);q(v4,c);i k0=L7(
#ifdef IB
C2,
#endif
#ifdef T
g3(g1),
#endif
X1 Y2);
#ifndef Q
i xb=K0(m0);
#endif
d y0=
#ifdef DB
j1;
#else
tb(O);
#endif
c C6=v4;
#ifndef Q
C6+=(xb.xy+xb.zw)*j.lh;
#endif
C6=floor(C6);uint M7=k3.y;uint Z1=k3.x+L8(N0(C6),M7);d I1=1.;
#ifdef AB
if(AB){d yb=m3(g5(O0));I1=min(yb,I1);}
#endif
#ifdef I
if(I&&Y1.x!=.0){d zb=K0(i0).x;I1=min(zb,I1);}
#endif
I1=max(I1,.0);y0=clamp(y0,.0,I1);uint o1;d O7;float P3;
#ifndef DB
if(V5(O)){O7=xi(k0.w,y0,Z1,o1,P3);}else
#endif
{O7=yi(k0.w,y0,Z1,o1,P3);}
#ifdef LB
d L5;if(LB){L5=ja(d0.xy,j.F3,j.G3);}
#endif
#ifdef Q
k0*=O7;
#else
if(T&&g3(g1)!=B4){k0.w*=O7;if(k0.w>.0){bool zi=o1>=j.f2&&(o1&i7)!=0u;if(!zi){k0.xyz=Y4(k0.xyz,xb,g3(g1));if(P3<1.){v P7=k0.xyz;
#ifdef LB
if(LB){P7+=L5*j.Fd;}
#endif
Wa(D6,E0(P7,.0));memoryBarrier();nh(S0,Z1,i7);}}else{k0.xyz=Va(D6).xyz;}}k0.xyz*=k0.w;}else{k0*=O7;}
#endif
#ifdef LB
k0.xyz=K2(k0.xyz,k0.w,L5);
#endif
A0(i0,E0(.0));d4(k0);}
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive