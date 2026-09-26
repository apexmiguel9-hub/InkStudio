#pragma once

#include "rhi.glsl.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char rhi[] = R"===(#pragma warning(disable:3550)
#pragma warning(disable:4000)
#ifndef _ARE_TOKEN_NAMES_PRESERVED
#define d half
#define D half2
#define v half3
#define i half4
#define N ushort
#define c float2
#define S float3
#define f float4
#define H4 bool2
#define o6 bool3
#define y7 bool4
#define N0 uint2
#define R uint4
#define Y int2
#define d6 int4
#define N ushort
#define e0 float2x2
#define Z6 half3x3
#define a7 half2x3
#define I4 half4x4
#endif
typedef S R3;
#ifdef UE
#ifdef DF
typedef min16uint N;
#endif
#else
#ifdef DF
typedef uint N;
#endif
#endif
#define fe(B,H) B##H
#define e inline
#define c1(o2) out o2
#define K6(o2) inout o2
#define h1(a) struct a{
#define K(g,Z,a) Z a:fe(rj,g)
#define i1 };
#define L(P8,F,a,Z) Z a=F.a
#define x7(g,a) cbuffer a{struct{
#define M8(a) }a;}
#define Ld(a) struct a{
#define Md(Z,a) Z a;
#define Nd(a) };[[vk::push_constant]]ConstantBuffer<Dh>a;
#define q2 struct q0{
#define I0 noperspective
#define MB nointerpolation
#define V2 nointerpolation
#define W(g,Z,a) Z a:fe(TEXCOORD,g)
#ifdef EF
#define i2 f R0:SV_Position;f Eh:SV_ClipDistance;};
#else
#define i2 f R0:SV_Position;};
#endif
#define V(a,Z) Z a
#define c0(a) f0.a=a
#define q(a,Z) Z a=f0.a
#ifdef CB
#define Y3
#define Z3
#endif
#ifdef EB
#define I3
#define J3
#endif
#define i5
#define j5
#define L4(a0,g,a) uniform Texture2D<R>a
#define p6(a0,g,a) uniform Texture2D<f>a
#ifdef SD
#define Df(a0,g,a) uniform Texture2DMS<i>a
#endif
#define e3(a0,g,a) uniform Texture2D<i>a
#define o5(a0,g,a) uniform Texture2D<d>a
#define h6(a0,g,a) uniform Texture2DArray<d>a
#define B5(g,a) SamplerState a;
#define g4 B5
#define q6(a0,g,a) B5(g,a)
#define c4(a) B5(a4,a)
#ifdef uj
#define m8(a,Fh,m) a.vj(m,Fh)
#endif
#define v1(a,m) a[m]
#define y5(a,p,m) a.Sample(p,m)
#define j2(a,p,m,U0) a.SampleLevel(p,m,U0)
#define z5(a,p,m,U1) a.SampleBias(p,m,U1)
#define Y6(a,p,E,r6,R8,U0) a.SampleLevel(p,S(E,0.5,r6),U0)
#define g8(n0,p,m) y5(n0,p,m)
#define U6(n0,p,m,U0) j2(n0,p,m,U0)
#define A7(n0,p,m,U1) z5(n0,p,m,U1)
#define z2
#define A2
#ifdef VE
#define Q2 RasterizerOrderedTexture2D
#else
#define Q2 RWTexture2D
#endif
#if defined(EB)&&defined(BB)
#ifdef ZF
#define Gh (G2+1)
#define p5(a) [[vk::input_attachment_index(Gh)]]SubpassInputMS<i>a
#define x6(a) na(I4(a.SubpassLoad(0),a.SubpassLoad(1),a.SubpassLoad(2),a.SubpassLoad(3)),Z8)
#elif defined(AG)
#define p5(a) Texture2DMS<i>a
#define x6(a) na(I4(a.Load(G,0),a.Load(G,1),a.Load(G,2),a.Load(G,3)),Z8)
#else
#define p5(a) Texture2D a
#define x6(a) a[G]
#endif
#endif
#define M1
#define N1
#ifdef SC
#define z0(g,a) uniform Q2<xj i>a
#else
#define z0(g,a) uniform Q2<uint>a
#endif
#define y4 z0
#define k1(g,a) uniform Q2<uint>a
#define Z2 a1
#define a3 d1
#if COMPILER_METAL||FORCE_ATOMIC_BUFFER
#define H2(g,a) uniform RWBuffer<uint>a
#define Z2(h) h[G0]
#define a3(h,C) h[G0]=C
#else
#define H2 k1
#define Z2 a1
#define a3 d1
#endif
#ifdef SC
#define K0(h) h[G]
#else
#define K0(h) unpackUnorm4x8(h[G])
#endif
#define a1(h) h[G]
#ifdef SC
#define A0(h,C) h[G]=(C)
#else
#define A0(h,C) h[G]=packUnorm4x8(C)
#endif
#define d1(h,C) h[G]=(C)
#if COMPILER_METAL||FORCE_ATOMIC_BUFFER
e uint C5(RWBuffer<uint>x3,uint G0,uint x){uint X0;InterlockedMax(x3[G0],x,X0);return X0;}
#define e5(h,E) C5(h,G0,E)
e uint D5(RWBuffer<uint>x3,uint G0,uint x){uint X0;InterlockedAdd(x3[G0],x,X0);return X0;}
#define f5(h,E) D5(h,G0,E)
#else
e uint C5(Q2<uint>x3,Y G,uint x){uint X0;InterlockedMax(x3[G],x,X0);return X0;}
#define e5(h,E) C5(h,G,E)
e uint D5(Q2<uint>x3,Y G,uint x){uint X0;InterlockedAdd(x3[G],x,X0);return X0;}
#define f5(h,E) D5(h,G,E)
#endif
#define y2(h)
#define h2(h)
#define k6
#define A3
#define L3
#define e1
#ifdef BG
#define db
#define eb(o4) (o4)
#else
#define db uint baseInstance;
#define eb(o4) ((o4)+baseInstance)
#endif
#ifdef FF
#define B1(a,g0,F,A,r) db f a(g0 F,uint A:SV_VertexID,uint o4:SV_InstanceID):SV_Position{uint r=eb(o4);
#define C1(E5) return E5;}
#else
#define B1(a,g0,F,A,r) db q0 a(g0 F,uint A:SV_VertexID,uint o4:SV_InstanceID){uint r=eb(o4);q0 f0;
#define S7(a,g0,F,p1,h0,A,r) q0 a(g0 F,p1 h0,uint A:SV_VertexID){q0 f0;f R0;
#define I6(a,n3,o3,C3,D3,p1,h0,A) q0 a(n3 o3,C3 D3,p1 h0,uint A:SV_VertexID){q0 f0;f R0;
#define C1(E5) f0.R0=E5;}return f0;
#endif
#if COMPILER_DXC&&(COMPILER_VULKAN||COMPILER_GLSL_ES3_1)
#define gb(fb) (fb)
#else
#define gb(fb) (!(fb))
#endif
#ifdef FF
#define f3(z1,a) EARLYDEPTHSTENCIL z1 a(f R0:SV_Position):SV_Target{c d0=R0.xy;
#define v6(z1,a) yj z1 a(f R0:SV_Position,uint Z8:SV_Coverage,bool hb:SV_IsFrontFace):SV_Target{c d0=R0.xy;bool w6=gb(hb);
#else
#define f3(z1,a) EARLYDEPTHSTENCIL z1 a(q0 f0,uint Z8:SV_Coverage):SV_Target{c d0=f0.R0.xy;Y G=Y(floor(d0));uint G0=G.y*j.n6+G.x;
#define v6(z1,a) z1 a(q0 f0,uint Z8:SV_Coverage,bool hb:SV_IsFrontFace):SV_Target{c d0=f0.R0.xy;Y G=Y(floor(d0));uint G0=G.y*j.n6+G.x;bool w6=gb(hb);
#endif
#define M2(C) return C;}
#ifdef EF
#define f7 ,out f gl_ClipDistance
#define A5 ,f0.Eh
#else
#define f7
#define A5
#endif
#define L6 ,c d0
#define Y2 ,d0
#define W3 ,Y G
#define Q1 ,G
#define P1(a) EARLYDEPTHSTENCIL void a(q0 f0){c d0=f0.R0.xy;Y G=Y(floor(d0));uint G0=G.y*j.n6+G.x;
#if defined(Q)&&defined(KB)
#define d2 r3
#else
#define d2 }
#endif
#define v2(a) EARLYDEPTHSTENCIL i a(q0 f0):SV_Target{c d0=f0.R0.xy;Y G=Y(floor(d0));uint G0=G.y*j.n6+G.x;i F1;
#define r3 }return F1;
#define uintBitsToFloat asfloat
#define floatBitsToInt asint
#define floatBitsToUint asuint
#define inversesqrt rsqrt
#define equal(B,H) ((B)==(H))
#define notEqual(B,H) ((B)!=(H))
#define lessThan(B,H) ((B)<(H))
#define greaterThan(B,H) ((B)>(H))
#define P0(B,H) mul(H,B)
#define F4
#define G4
#define U3
#define V3
#define O5(g,y1,a) StructuredBuffer<N0>a
#define M4(g,y1,a) StructuredBuffer<R>a
#define P5(g,y1,a) StructuredBuffer<f>a
#define L0(a,C0) a[C0]
#define R5(a,C0) a[C0]
e D unpackHalf2x16(uint u){uint y=(u>>16);uint x=u&0xffffu;return D(f16tof32(x),f16tof32(y));}e uint packHalf2x16(c m2){uint x=f32tof16(m2.x);uint y=f32tof16(m2.y);return(y<<16)|x;}e i unpackUnorm4x8(uint u){R l1=R(u&0xffu,(u>>8)&0xffu,(u>>16)&0xffu,u>>24);return i(l1)*(1./255.);}e c unpackUnorm2x16(uint u){N0 l1=N0(u&0xffffu,u>>16);return c(l1)*(1./65535.);}e uint packUnorm4x8(i k){R l1=(R(saturate(k)*255.)&0xff)<<R(0,8,16,24);l1.xy|=l1.zw;l1.x|=l1.y;return l1.x;}e e0 inverse(e0 n1){e0 Za=e0(n1[1][1],-n1[0][1],-n1[1][0],n1[0][0]);return Za*(1./determinant(n1));}e float mix(float x,float y,float s){return lerp(x,y,s);}e c mix(c x,c y,c s){return lerp(x,y,s);}e S mix(S x,S y,S s){return lerp(x,y,s);}e f mix(f x,f y,f s){return lerp(x,y,s);}e float fract(float x){return frac(x);}e c fract(c x){return frac(x);}e S fract(S x){return frac(x);}e f fract(f x){return frac(x);}e float mod(float x,float y){return fmod(x,y);}e float R2(float x){return sign(x);}e c R2(c x){return sign(x);}e S R2(S x){return sign(x);}e f R2(f x){return sign(x);}
#define sign R2
e float S2(float x){return abs(x);}e c S2(c x){return abs(x);}e S S2(S x){return abs(x);}e f S2(f x){return abs(x);}
#define abs S2
e float T2(float x){return sqrt(x);}e c T2(c x){return sqrt(x);}e S T2(S x){return sqrt(x);}e f T2(f x){return sqrt(x);}
#define sqrt T2
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive