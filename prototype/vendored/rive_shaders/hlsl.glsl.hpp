#pragma once

#include "hlsl.glsl.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char hlsl[] = R"===(#pragma warning(disable:3550)
#pragma warning(disable:4000)
#ifndef _ARE_TOKEN_NAMES_PRESERVED
#define d half
#define D half2
#define v half3
#define i half4
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
#define e0 float2x2
#define Z6 half3x3
#define a7 half2x3
#define I4 half4x4
#endif
typedef S R3;
#ifdef UE
#define N min16uint
#else
#define N uint
#endif
#define e inline
#define c1(o2) out o2
#define K6(o2) inout o2
#define h1(a) struct a{
#define K(g,Z,a) Z a:a
#define i1 };
#define L(P8,F,a,Z) Z a=F.a
#define be(g) register(b##g)
#define x7(g,a) cbuffer a:be(g){struct{
#define M8(a) }a;}
#define q2 struct q0{
#define I0 noperspective
#define MB nointerpolation
#define V2 nointerpolation
#define W(g,Z,a) Z a:TEXCOORD##g
#define i2 f R0:SV_Position;};
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
#define L4(a0,g,a) uniform Texture2D<R>a:register(t##g)
#define p6(a0,g,a) uniform Texture2D<f>a:register(t##g)
#define e3(a0,g,a) uniform Texture2D<unorm f>a:register(t##g)
#define o5(a0,g,a) uniform Texture2D<d>a:register(t##g)
#define h6(a0,g,a) uniform Texture1DArray<d>a:register(t##g)
#define B5(g,a) SamplerState a:register(s##g);
#define g4 B5
#define q6(a0,g,a) B5(g,a)
#define c4(a) B5(a4,a)
#define v1(a,m) a[m]
#define y5(a,p,m) a.Sample(p,m)
#define j2(a,p,m,U0) a.SampleLevel(p,m,U0)
#define z5(a,p,m,U1) a.SampleBias(p,m,U1)
#define Y6(a,p,E,r6,R8,U0) a.SampleLevel(p,c(E,r6),U0)
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
#define M1
#ifdef SC
#define z0(g,a) uniform Q2<unorm i>a:register(u##g)
#else
#define z0(g,a) uniform Q2<uint>a:register(u##g)
#endif
#define y4 z0
#define k1(g,a) uniform Q2<uint>a:register(u##g)
#define H2 k1
#define Z2 a1
#define a3 d1
#define N1
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
e uint C5(Q2<uint>x3,Y G,uint x){uint X0;InterlockedMax(x3[G],x,X0);return X0;}
#define e5(h,E) C5(h,G,E)
e uint D5(Q2<uint>x3,Y G,uint x){uint X0;InterlockedAdd(x3[G],x,X0);return X0;}
#define f5(h,E) D5(h,G,E)
#define y2(h)
#define h2(h)
#define k6
#define A3
#define L3
#define e1
#define f7
#define A5
#define B1(a,g0,F,A,r) cbuffer lj:be(Xc){uint th;uint a##mj;uint a##nj;uint a##oj;}q0 main(g0 F,uint A:SV_VertexID,uint o4:SV_InstanceID){uint r=o4+th;q0 f0;
#define S7(a,g0,F,p1,h0,A,r) q0 main(g0 F,p1 h0,uint A:SV_VertexID){q0 f0;f R0;
#define I6(a,n3,o3,C3,D3,p1,h0,A) q0 main(n3 o3,C3 D3,p1 h0,uint A:SV_VertexID){q0 f0;f R0;
#define C1(E5) f0.R0=E5;}return f0;
#define f3(z1,a) z1 main(q0 f0):SV_Target{
#define v6(z1,a) z1 main(q0 f0,bool w6:SV_IsFrontFace):SV_Target{
#define M2(C) return C;}
#define L6 ,c d0
#define Y2 ,d0
#define W3 ,Y G
#define Q1 ,G
#define P1(a) [earlydepthstencil]void main(q0 f0){c d0=f0.R0.xy;Y G=Y(floor(d0));
#define d2 }
#define v2(a) [earlydepthstencil]i main(q0 f0):SV_Target{c d0=f0.R0.xy;Y G=Y(floor(d0));i F1;
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
#define O5(g,y1,a) StructuredBuffer<N0>a:register(t##g)
#define M4(g,y1,a) StructuredBuffer<R>a:register(t##g)
#define P5(g,y1,a) StructuredBuffer<f>a:register(t##g)
#define L0(a,C0) a[C0]
#define R5(a,C0) a[C0]
e D unpackHalf2x16(uint u){uint y=(u>>16);uint x=u&0xffffu;return D(f16tof32(x),f16tof32(y));}e uint packHalf2x16(c m2){uint x=f32tof16(m2.x);uint y=f32tof16(m2.y);return(y<<16)|x;}e i unpackUnorm4x8(uint u){R l1=R(u&0xffu,(u>>8)&0xffu,(u>>16)&0xffu,u>>24);return i(l1)*(1./255.);}e c unpackUnorm2x16(uint u){N0 l1=N0(u&0xffffu,u>>16);return c(l1)*(1./65535.);}e uint packUnorm4x8(i k){R l1=(R(saturate(k)*255.)&0xff)<<R(0,8,16,24);l1.xy|=l1.zw;l1.x|=l1.y;return l1.x;}e e0 inverse(e0 n1){e0 Za=e0(n1[1][1],-n1[0][1],-n1[1][0],n1[0][0]);return Za*(1./determinant(n1));}e float mix(float x,float y,bool s){return s?y:x;}e c mix(c x,c y,H4 s){return s?y:x;}e S mix(S x,S y,o6 s){return s?y:x;}e f mix(f x,f y,y7 s){return s?y:x;}e d mix(d x,d y,bool s){return s?y:x;}e D mix(D x,D y,H4 s){return s?y:x;}e v mix(v x,v y,o6 s){return s?y:x;}e i mix(i x,i y,y7 s){return s?y:x;}e float mix(float x,float y,float s){return lerp(x,y,s);}e c mix(c x,c y,c s){return lerp(x,y,s);}e S mix(S x,S y,S s){return lerp(x,y,s);}e f mix(f x,f y,f s){return lerp(x,y,s);}e d mix(d x,d y,d s){return lerp(x,y,s);}e D mix(D x,D y,D s){return lerp(x,y,s);}e v mix(v x,v y,v s){return lerp(x,y,s);}e i mix(i x,i y,i s){return lerp(x,y,s);}e float fract(float x){return frac(x);}e c fract(c x){return frac(x);}e S fract(S x){return frac(x);}e f fract(f x){return frac(x);}e d fract(d x){return frac(x);}e D fract(D x){return D(frac(x));}e v fract(v x){return v(frac(x));}e i fract(i x){return i(frac(x));}e float mod(float x,float y){return fmod(x,y);}e d R2(d x){return sign(x);}e D R2(D x){return D(sign(x));}e v R2(v x){return v(sign(x));}e i R2(i x){return i(sign(x));}e float R2(float x){return sign(x);}e c R2(c x){return sign(x);}e S R2(S x){return sign(x);}e f R2(f x){return sign(x);}
#define sign R2
e d S2(d x){return abs(x);}e D S2(D x){return D(abs(x));}e v S2(v x){return v(abs(x));}e i S2(i x){return i(abs(x));}e float S2(float x){return abs(x);}e c S2(c x){return abs(x);}e S S2(S x){return abs(x);}e f S2(f x){return abs(x);}
#define abs S2
e d T2(d x){return sqrt(x);}e D T2(D x){return D(sqrt(x));}e v T2(v x){return v(sqrt(x));}e i T2(i x){return i(sqrt(x));}e float T2(float x){return sqrt(x);}e c T2(c x){return sqrt(x);}e S T2(S x){return sqrt(x);}e f T2(f x){return sqrt(x);}
#define sqrt T2
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive