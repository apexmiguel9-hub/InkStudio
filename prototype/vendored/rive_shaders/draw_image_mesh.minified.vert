#ifdef VERTEX
h1(n3) K(0,c,OC);i1 h1(C3) K(1,c,PC);i1 h1(p1) K(v9,f,XB);K(w9,f,RB);K(x9,f,NB);K(y9,uint,YB);K(z9,uint,ZB);K(A9,uint,AC);K(B9,uint,LC);i1
#endif
q2 I0 W(0,c,J5);
#ifdef ENABLE_CLIPPING
OPTIONALLY_FLAT W(1,d,O3);
#endif
#if defined(ENABLE_CLIP_RECT)&&!defined(RENDER_MODE_DEPTH_STENCIL)
I0 W(2,f,O0);
#endif
OPTIONALLY_FLAT W(3,i,K1);
#ifdef ENABLE_ADVANCED_BLEND
V2 W(4,N,D1);
#endif
i2
#ifdef VERTEX
Y3 Z3 I6(EC,n3,o3,C3,D3,p1,h0,A){L(A,o3,OC,c);L(A,D3,PC,c);L(r,h0,XB,f);L(r,h0,RB,f);L(r,h0,NB,f);L(r,h0,YB,uint);L(r,h0,ZB,uint);L(r,h0,AC,uint);L(r,h0,LC,uint);V(J5,c);
#ifdef ENABLE_CLIPPING
V(O3,d);
#endif
#if defined(ENABLE_CLIP_RECT)&&!defined(RENDER_MODE_DEPTH_STENCIL)
V(O0,f);
#endif
V(K1,i);
#ifdef ENABLE_ADVANCED_BLEND
V(D1,N);
#endif
c l0=P0(L1(XB),OC)+NB.xy;J5=PC;
#ifdef ENABLE_CLIPPING
if(ENABLE_CLIPPING){O3=r8(ZB,j.c6);}
#endif
#ifdef ENABLE_CLIP_RECT
if(ENABLE_CLIP_RECT){
#ifndef RENDER_MODE_DEPTH_STENCIL
O0=T7(L1(RB),NB.zw,l0 A5);
#else
Mc(L1(RB),NB.zw,l0 A5);
#endif
}
#endif
f X=Q3(l0);
#ifdef POST_INVERT_Y
X.y=-X.y;
#endif
#ifdef RENDER_MODE_DEPTH_STENCIL
X.z=ma(LC,0xffu);
#endif
K1=unpackUnorm4x8(YB);
#ifdef ENABLE_ADVANCED_BLEND
D1=a2(AC);
#endif
c0(J5);
#ifdef ENABLE_CLIPPING
c0(O3);
#endif
#if defined(ENABLE_CLIP_RECT)&&!defined(RENDER_MODE_DEPTH_STENCIL)
c0(O0);
#endif
c0(K1);
#ifdef ENABLE_ADVANCED_BLEND
c0(D1);
#endif
C1(X);}
#endif
