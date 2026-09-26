#pragma once

#include "atomic_draw.glsl.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char atomic_draw[] = R"===(#ifdef ND
#ifdef CB
h1(g0) K(0,f,VB);K(1,f,WB);i1
#endif
q2
#ifdef GB
I0 W(0,f,O);
#else
I0 W(0,D,O);
#endif
V2 W(1,N,D0);i2
#ifdef CB
B1(EC,g0,F,A,r){L(A,F,VB,f);L(A,F,WB,f);
#ifdef GB
V(O,f);
#else
V(O,D);
#endif
V(D0,N);f X;uint o0;c l0;f P;if(r9(VB,WB,r,o0,l0,P A3)){
#ifdef GB
O=P;
#else
O.xy=R7(P.xy);
#endif
D0=a2(o0);X=Q3(l0);}else{X=f(j.W2,j.W2,j.W2,j.W2);}c0(O);c0(D0);C1(X);}
#endif
#endif
#if defined(DB)||defined(FB)
#ifdef CB
h1(g0) K(0,R3,JB);i1
#endif
q2
#ifdef FB
I0 W(0,c,F2);
#else
MB W(0,d,j1);
#endif
V2 W(1,N,D0);i2
#ifdef CB
B1(EC,g0,F,A,r){L(A,F,JB,S);
#ifdef FB
V(F2,c);
#else
V(j1,d);
#endif
V(D0,N);uint o0;c l0;
#ifdef FB
l0=Jb(JB,o0,F2 A3);
#else
l0=Kb(JB,o0,j1 A3);
#endif
D0=a2(o0);f X=Q3(l0);
#ifdef FB
c0(F2);
#else
c0(j1);
#endif
c0(D0);C1(X);}
#endif
#endif
#ifdef ZC
#ifdef CB
h1(g0) K(0,f,FC);i1 h1(p1) K(v9,f,XB);K(w9,f,RB);K(x9,f,NB);K(y9,uint,YB);K(z9,uint,ZB);K(A9,uint,AC);K(B9,uint,LC);K(cf,f,OD);K(df,f,PD);K(ef,f,AD);K(Lb,f,NC);i1
#endif
q2 I0 W(0,c,c2);I0 W(1,d,Z4);I0 W(2,f,a5);
#ifdef AB
I0 W(3,f,O0);
#endif
MB W(4,i,K1);
#ifdef I
V2 W(5,N,B3);
#endif
#ifdef T
V2 W(6,N,D1);
#endif
i2
#ifdef CB
S7(EC,g0,F,p1,h0,A,r){L(A,F,FC,f);L(r,h0,XB,f);L(r,h0,RB,f);L(r,h0,NB,f);L(r,h0,YB,uint);L(r,h0,ZB,uint);L(r,h0,AC,uint);L(r,h0,LC,uint);L(r,h0,OD,f);L(r,h0,PD,f);L(r,h0,AD,f);L(r,h0,NC,f);V(c2,c);V(Z4,d);V(a5,f);
#ifdef AB
V(O0,f);
#endif
V(K1,i);
#ifdef I
V(B3,N);
#endif
#ifdef T
V(D1,N);
#endif
bool C9=FC.z==.0||FC.w==.0;Z4=C9?.0:1.;c l0=FC.xy;e0 W0=L1(XB);e0 H6=transpose(inverse(W0));if(!C9){float D9=x4*E9(H6[1])/dot(W0[1],H6[1]);if(D9>=.5){l0.x=.5;Z4*=S3(.5/D9);}else{l0.x+=D9*FC.z;}float F9=x4*E9(H6[0])/dot(W0[0],H6[0]);if(F9>=.5){l0.y=.5;Z4*=S3(.5/F9);}else{l0.y+=F9*FC.w;}}e0 ff=L1(OD);c2=P0(ff,l0)+AD.xy;l0=P0(W0,l0)+NB.xy;if(C9){c T3=P0(H6,FC.zw);T3*=E9(T3)/dot(T3,T3);l0+=x4*T3;}
#ifdef AB
if(AB){O0=T7(L1(RB),NB.zw,l0);}
#endif
K1=unpackUnorm4x8(YB);
#ifdef I
B3=a2(ZB);
#endif
#ifdef T
D1=a2(AC);
#endif
f X=Q3(l0);c v0=l0;
#ifdef ME
if(j.Mb!=0u){v0.y=float(j.Nb)-v0.y;}
#endif
if(NC.w!=0.0){e0 gf=L1(PD);c hf=AD.zw;a5=Ob(v0,gf,hf,NC.w,NC.xy,NC.z);}else{a5=f(.0,.0,.0,.0);}c0(c2);c0(Z4);c0(a5);
#ifdef AB
c0(O0);
#endif
c0(K1);
#ifdef I
c0(B3);
#endif
#ifdef T
c0(D1);
#endif
C1(X);}
#endif
#elif defined(KB)
#ifdef CB
h1(n3) K(0,c,OC);i1 h1(C3) K(1,c,PC);i1 h1(p1) K(v9,f,XB);K(w9,f,RB);K(x9,f,NB);K(y9,uint,YB);K(z9,uint,ZB);K(A9,uint,AC);K(B9,uint,LC);i1
#endif
q2 I0 W(0,c,c2);
#ifdef AB
I0 W(1,f,O0);
#endif
MB W(3,i,K1);
#ifdef I
V2 W(4,N,B3);
#endif
#ifdef T
V2 W(5,N,D1);
#endif
i2
#ifdef CB
I6(EC,n3,o3,C3,D3,p1,h0,A){L(A,o3,OC,c);L(A,D3,PC,c);L(r,h0,XB,f);L(r,h0,RB,f);L(r,h0,NB,f);L(r,h0,YB,uint);L(r,h0,ZB,uint);L(r,h0,AC,uint);L(r,h0,LC,uint);V(c2,c);
#ifdef AB
V(O0,f);
#endif
V(K1,i);
#ifdef I
V(B3,N);
#endif
#ifdef T
V(D1,N);
#endif
e0 W0=L1(XB);c l0=P0(W0,OC)+NB.xy;c2=PC;
#ifdef AB
if(AB){O0=T7(L1(RB),NB.zw,l0);}
#endif
K1=unpackUnorm4x8(YB);
#ifdef I
B3=a2(ZB);
#endif
#ifdef T
D1=a2(AC);
#endif
f X=Q3(l0);c0(c2);
#ifdef AB
c0(O0);
#endif
c0(K1);
#ifdef I
c0(B3);
#endif
#ifdef T
c0(D1);
#endif
C1(X);}
#endif
#endif
#ifdef HF
#ifdef CB
h1(g0) i1
#endif
q2 i2
#ifdef CB
B1(EC,g0,F,A,r){Y r2;r2.x=(A&1)==0?j.U7.x:j.U7.z;r2.y=(A&2)==0?j.U7.y:j.U7.w;f X=Q3(c(r2));C1(X);}
#endif
#endif
#ifdef NE
#endif
#if defined(OE)&&!defined(Q)
#endif
#ifdef EB
M1
#ifndef Q
#ifdef PE
#define G9 PE
#else
#define G9 G2
#endif
#ifdef BD
y4(G9,m0);
#else
z0(G9,m0);
#endif
#endif
#ifdef VC
#define z4 i
#define H9 K0
#define V7 E0(.0)
#define Pb(E) ((E).w!=.0)
#ifdef I
#ifndef QC
z0(X2,i0);
#else
y4(X2,i0);
#endif
#endif
#else
#define z4 uint
#define V7 0u
#define H9 a1
#define Pb(E) ((E)!=0u)
#ifdef I
k1(X2,i0);
#endif
#endif
H2(J6,A4);N1 U3 O5(Qb,kf,CD);P5(Rb,lf,PB);V3 e uint mf(float x){return uint(round(x*I9+J9));}e d W7(uint x){return S3(float(x)*Sb+(-J9*Sb));}N X7(N o0){
#ifdef IF
o0=min(o0,j.nf);
#endif
return o0;}
#ifdef I
e void Tb(uint m1,z4 Q0,K6(d) o){
#ifdef VC
if(all(lessThan(abs(Q0.xy-unpackUnorm4x8(m1).xy),D2(.25/255.)))) o=min(o,Q0.z);else o=.0;
#else
if(m1==Q0>>16) o=min(o,unpackHalf2x16(Q0).x);else o=.0;
#endif
}
#endif
e void Y7(uint o0,d r0,c1(i) J
#if defined(I)&&!defined(QC)
,K6(z4) q1
#endif
L6 W3){N0 r1=R5(CD,o0);d o=r0;if((r1.x&(of|K9))!=0u){o=abs(o);
#ifdef WC
if(WC&&(r1.x&K9)!=0u){o=1.-abs(fract(o*.5)*2.+-1.);}
#endif
}o=clamp(o,J0(.0),J0(1.));
#ifdef I
if(I){uint m1=r1.x>>16u;if(m1!=0u){Tb(m1,H9(i0),o);}}
#endif
#ifdef AB
if(AB&&(r1.x&pf)!=0u){e0 W0=L1(L0(PB,o0*E3+2u));f I2=L0(PB,o0*E3+3u);c qf=P0(W0,d0)+I2.xy;D Ub=R7(abs(qf)*I2.zw-I2.zw);d c5=clamp(min(Ub.x,Ub.y)+.5,.0,1.);o=min(o,c5);}
#endif
uint X3=r1.x&0xfu;N p3=a2((r1.x>>4)&0xfu);
#ifdef T
bool d5=T&&p3!=B4;
#else
const bool d5=false;
#endif
if(X3<=Vb){J=unpackUnorm4x8(r1.y);
#ifdef I
if(I&&X3==Z7){
#ifndef QC
#ifdef VC
q1.xy=J.zw;q1.z=o;q1.w=1.;
#else
q1=r1.y|packHalf2x16(D2(o,.0));
#endif
#endif
J=E0(.0);}
#endif
}else{e0 W0=L1(L0(PB,o0*E3));f I2=L0(PB,o0*E3+1u);c Wb=P0(W0,d0)+I2.xy;float t=X3==Xb?Wb.x:length(Wb);t=clamp(t,.0,1.);float x=t*I2.z+I2.w;float Yb=uintBitsToFloat(r1.y);float rf=floor(Yb)*j.Zb+j.ac;J=j2(DD,M9,c(x,rf),.0);if(!d5){J.xyz*=J.w;d N9=S3(fract(Yb)*(256./255.));J.w*=N9;}}
#if!defined(Q)&&defined(T)
if(d5){if(J.w*o!=.0){i O1=K0(m0);J.xyz=Y4(J.xyz,O1,p3);}J.xyz*=J.w;}
#endif
J*=o;
#if defined(BC)&&(defined(Q)||defined(QC))
J=q3(J);
#endif
}
#if!defined(Q)&&!defined(BD)
e void a8(i J W3){
#ifndef VC
if(J.x+J.y+J.z+J.w==.0) return;float M6=1.-J.w;if(M6!=.0) J+=K0(m0)*M6;
#endif
A0(m0,J);}
#endif
#if defined(I)&&!defined(QC)
e void O9(z4 q1 W3){
#ifdef VC
A0(i0,q1);
#else
if(q1!=0u) d1(i0,q1);
#endif
}
#endif
#ifdef Q
#define S5 v2
#define T5 r3
#else
#define S5 P1
#define T5 d2
#endif
#ifdef ND
S5(HB){
#ifdef GB
q(O,f);
#else
q(O,D);
#endif
q(D0,N);d c8;
#ifdef GB
if(GB&&bc(O)){c8=C4(O e1);}else if(GB&&cc(O)){c8=d8(O e1);}else
#endif
{c8=min(min(J0(O.x),abs(J0(O.y))),J0(1.));}i J=E0(.0);
#ifdef I
z4 q1=V7;
#endif
uint e8=mf(c8);uint dc=(ec(D0)<<U5)|e8;uint w2=e5(A4,dc);N E1=a2(w2>>U5);E1=X7(E1);if(E1==D0){if(!V5(O)){e8+=w2-max(dc,w2);e8-=P9;f5(A4,e8);}}else{d r0=W7(w2&f8);Y7(E1,r0,J
#ifdef I
,q1
#endif
Y2 Q1);}J.xyz=K2(J.xyz,J.w,d0.xy,j.F3,j.G3);
#ifdef Q
F1=J;
#else
a8(J Q1);
#endif
#ifdef I
O9(q1 Q1);
#endif
T5}
#endif
#if defined(DB)||defined(FB)
S5(HB){
#ifdef FB
q(F2,c);
#else
q(j1,d);
#endif
q(D0,N);uint w2=Z2(A4);N E1=a2(w2>>U5);E1=X7(E1);uint Q9;
#ifndef FB
if(E1==D0){Q9=w2;}else
#endif
{Q9=(ec(D0)<<U5)+P9;}d o;
#ifdef FB
o=clamp(j2(ED,R9,F2,.0).x,J0(.0),J0(1.));
#else
o=j1;
#endif
int sf=int(round(o*I9));a3(A4,Q9+uint(sf));i J=E0(.0);
#ifdef I
z4 q1=V7;
#endif
#ifndef FB
if(E1!=D0)
#endif
{d S9=W7(w2&f8);Y7(E1,S9,J
#ifdef I
,q1
#endif
Y2 Q1);}J.xyz=K2(J.xyz,J.w,d0.xy,j.F3,j.G3);
#ifdef Q
F1=J;
#else
a8(J Q1);
#endif
#ifdef I
O9(q1 Q1);
#endif
T5}
#endif
#ifdef NE
S5(HB){q(c2,c);
#ifdef ZC
q(Z4,d);q(a5,f);
#endif
#ifdef AB
q(O0,f);
#endif
q(K1,i);
#ifdef I
q(B3,N);
#endif
#ifdef T
q(D1,N);
#endif
i k2=g8(GC,W5,c2);d X5=1.;
#ifdef ZC
X5=min(Z4,X5);
#endif
#ifdef AB
if(AB){d c5=m3(g5(O0));X5=clamp(c5,J0(.0),X5);}
#endif
uint w2=Z2(A4);N E1=a2(w2>>U5);E1=X7(E1);d S9=W7(w2&f8);i J;
#ifdef I
z4 q1=V7;
#endif
Y7(E1,S9,J
#ifdef I
,q1
#endif
Y2 Q1);
#ifdef I
if(I&&B3!=0u){z4 Q0=Pb(q1)?q1:H9(i0);Tb(B3,Q0,X5);}
#endif
#ifdef ZC
if(a5.w!=0.0){c T9=fc(a5);i U9=j2(DD,M9,T9,0.0);U9.xyz*=U9.w;k2*=U9;}
#endif
k2*=K1;
#if!defined(Q)&&defined(T)
if(T&&D1!=B4){i O1=K0(m0)*(1.-J.w)+J;k2.xyz=Y4(F6(k2),O1,D1)*k2.w;}
#endif
k2*=X5;
#if defined(BC)
k2=q3(k2);
#endif
J=J*(1.-k2.w)+k2;J.xyz=K2(J.xyz,J.w,d0.xy,j.F3,j.G3);
#ifdef Q
F1=J;
#else
a8(J Q1);
#endif
#ifdef I
O9(q1 Q1);
#endif
a3(A4,P9);T5}
#endif
#ifdef OE
S5(HB){
#ifndef Q
#ifdef QD
if(QD){A0(m0,unpackUnorm4x8(j.tf));}
#endif
#ifdef RD
if(RD){A0(m0,v1(GC,G));}
#endif
#ifdef JF
i k=K0(m0);A0(m0,k.zyxw);
#endif
#endif
a3(A4,j.uf);
#ifdef I
if(I){d1(i0,0u);}
#endif
#ifdef Q
discard;
#endif
T5}
#endif
#ifdef QC
#ifdef BD
v2(HB)
#else
S5(HB)
#endif
{uint w2=Z2(A4);d r0=W7(w2&f8);N E1=a2(w2>>U5);E1=X7(E1);i J;Y7(E1,r0,J Y2 Q1);
#ifdef BD
float M6=1.-J.w;if(M6!=.0) J+=K0(m0)*M6;F1=J;r3
#else
J.xyz=K2(J.xyz,J.w,d0.xy,j.F3,j.G3);
#ifdef Q
F1=J;
#else
a8(J Q1);
#endif
T5
#endif
}
#endif
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive