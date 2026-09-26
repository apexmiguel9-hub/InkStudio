#ifdef FRAGMENT
#if(defined(FIXED_FUNCTION_COLOR_OUTPUT)&&!defined(ENABLE_CLIPPING))||defined(RENDER_MODE_CLOCKWISE_ATOMIC)
#undef Bb
#else
#define Bb
#endif
M1
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
z0(G2,m0);
#endif
#ifndef RENDER_MODE_CLOCKWISE_ATOMIC
k1(X2,i0);
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
z0(f6,p4);
#endif
k1(J6,S0);
#else
z0(X2,i0);
#endif
N1
#ifdef DRAW_IMAGE_MESH
I3 e3(h5,a4,GC);J3 i5 c4(W5) j5 U3 V3
#endif
#ifdef FIXED_FUNCTION_COLOR_OUTPUT
#ifdef DRAW_IMAGE_MESH
v2(HB)
#else
v2(HB)
#endif
#else
#ifdef DRAW_IMAGE_MESH
P1(HB)
#else
P1(HB)
#endif
#endif
{
#ifdef FEATHER_ATLAS_BLIT
q(X1,f);
#if defined(ENABLE_MODULATED_IMAGE)
q(C2,S);
#endif
q(F2,c);
#endif
#ifdef ENABLE_CLIPPING
q(O3,d);
#endif
#ifdef ENABLE_CLIP_RECT
q(O0,f);
#endif
#if defined(FEATHER_ATLAS_BLIT)&&defined(ENABLE_ADVANCED_BLEND)
q(g1,d);
#endif
#ifdef DRAW_IMAGE_MESH
q(J5,c);q(K1,i);
#ifdef ENABLE_ADVANCED_BLEND
q(D1,N);
#endif
#endif
#ifdef FEATHER_ATLAS_BLIT
i k=L7(
#ifdef ENABLE_MODULATED_IMAGE
C2,
#endif
#ifdef ENABLE_ADVANCED_BLEND
g3(g1),
#endif
X1 Y2);d o=clamp(j2(ED,R9,F2,.0).x,J0(.0),J0(1.));
#endif
#ifdef DRAW_IMAGE_MESH
i k=A7(GC,W5,J5,j.Ed);d o=1.;
#endif
#ifdef ENABLE_CLIP_RECT
if(ENABLE_CLIP_RECT){d c5=max(m3(g5(O0)),J0(.0));o=min(c5,o);}
#endif
#ifdef Bb
z2;
#endif
#if defined(ENABLE_CLIPPING)
if(ENABLE_CLIPPING&&O3!=.0){d z3;
#ifndef RENDER_MODE_CLOCKWISE_ATOMIC
D Q0=unpackHalf2x16(a1(i0));d E6=Q0.y;z3=max(E6==O3?Q0.x:J0(.0),J0(.0));
#else
z3=K0(i0).x;
#endif
z3=max(z3,J0(.0));o=min(o,z3);}
#endif
#ifdef DRAW_IMAGE_MESH
k*=K1;
#endif
#if!defined(FIXED_FUNCTION_COLOR_OUTPUT)
i O1=K0(m0);
#ifdef ENABLE_ADVANCED_BLEND
#ifdef FEATHER_ATLAS_BLIT
N p3=g3(g1);
#endif
#ifdef DRAW_IMAGE_MESH
N p3=D1;
#endif
if(ENABLE_ADVANCED_BLEND&&p3!=B4){
#ifdef DRAW_IMAGE_MESH
k.xyz=F6(k);
#endif
k.xyz=Y4(k.xyz,O1,p3)*k.w;}
#endif
k*=o;
#ifdef NEEDS_GAMMA_CORRECTION
if(NEEDS_GAMMA_CORRECTION){k=q3(k);}
#endif
k.xyz=K2(k.xyz,k.w,d0.xy,j.F3,j.G3);
#ifndef RENDER_MODE_CLOCKWISE_ATOMIC
k=O1*(1.-k.w)+k;
#endif
A0(m0,k);
#endif
#ifndef RENDER_MODE_CLOCKWISE_ATOMIC
h2(i0);h2(S0);
#else
A0(i0,E0(.0));
#endif
#ifdef Bb
A2;
#endif
#ifdef FIXED_FUNCTION_COLOR_OUTPUT
k=(k*o);k.xyz=K2(k.xyz,k.w,d0.xy,j.F3,j.G3);F1=k;r3
#else
d2;
#endif
}
#endif
