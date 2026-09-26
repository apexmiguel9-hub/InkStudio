#ifndef _ARE_TOKEN_NAMES_PRESERVED
#define d half
#define D half2
#define v half3
#define i half4
#define N ushort
#define c float2
#define S float3
#define R3 packed_float3
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
#define e inline
#define c1(o2) thread o2&
#define K6(o2) thread o2&
#define equal(B,H) ((B)==(H))
#define notEqual(B,H) ((B)!=(H))
#define lessThan(B,H) ((B)<(H))
#define greaterThan(B,H) ((B)>(H))
#define P0(B,H) ((B)*(H))
#define inversesqrt rsqrt
#define x7(g,a) struct a{
#define M8(a) };
#define h1(a) struct a{
#define K(g,Z,a) Z a
#define i1 };
#define L(P8,F,a,Z) Z a=F[P8].a
#define q2 struct q0{
#define W(g,Z,a) Z a
#define V2 [[flat]]
#define I0 [[center_no_perspective]]
#ifndef OPTIONALLY_FLAT
#define OPTIONALLY_FLAT
#endif
#define i2 f R0[[position]][[invariant]];};
#define V(a,Z) thread Z&a=f0.a
#define c0(a)
#define q(a,Z) Z a=f0.a
#define F4 struct U8{
#define G4 };
#define U3 struct V8{
#define V3 };
#define O5(g,y1,a) constant N0*a[[buffer(R1(g))]]
#define M4(g,y1,a) constant R*a[[buffer(R1(g))]]
#define P5(g,y1,a) constant f*a[[buffer(R1(g))]]
#define L0(a,C0) y3.a[C0]
#define R5(a,C0) y3.a[C0]
#define Y3 struct W8{
#define Z3 };
#define I3 struct F5{
#define J3 };
#define i5 struct ab{
#define j5 };
#define L4(a0,g,a) [[texture(g)]]texture2d<uint>a
#define p6(a0,g,a) [[texture(g)]]texture2d<float>a
#define e3(a0,g,a) [[texture(g)]]texture2d<d>a
#define o5(a0,g,a) [[texture(g)]]texture2d<d>a
#define h6(a0,g,a) [[texture(g)]]texture1d_array<d>a
#define g4(z7,a) constexpr sampler a(filter::linear,mip_filter::none);
#define q6(a0,g,a) [[sampler(g)]]sampler a;
#define c4(a) [[sampler(a4)]]sampler a;
#define v1(n0,m) Y0.n0.read(N0(m))
#define y5(n0,p,m) Y0.n0.sample(p,m)
#define j2(n0,p,m,U0) Y0.n0.sample(p,m,level(U0))
#define z5(n0,p,m,U1) Y0.n0.sample(p,m,bias(U1))
#define g8(n0,p,m) Y0.n0.sample(y6.p,m)
#define U6(n0,p,m,U0) Y0.n0.sample(y6.p,m,level(U0))
#define A7(n0,p,m,U1) Y0.n0.sample(y6.p,m,bias(U1))
#define Y6(n0,p,E,r6,R8,U0) Y0.n0.sample(p,E,r6)
#define k6 ,constant TB&j,W8 Y0,U8 y3
#define A3 ,j,Y0,y3
#ifdef ENABLE_INSTANCE_INDEX
#define B1(a,g0,F,A,r) __attribute__((visibility("default"))) q0 vertex a(uint A[[vertex_id]],uint r[[instance_id]],constant uint&uh[[buffer(R1(Xc))]],constant TB&j[[buffer(R1(K4))]],constant g0*F[[buffer(0)]],W8 Y0,U8 y3){r+=uh;q0 f0;
#else
#define B1(a,g0,F,A,r) __attribute__((visibility("default"))) q0 vertex a(uint A[[vertex_id]],uint r[[instance_id]],constant TB&j[[buffer(R1(K4))]],constant g0*F[[buffer(0)]],W8 Y0,U8 y3){q0 f0;
#endif
#define S7(a,g0,F,p1,h0,A,r) __attribute__((visibility("default"))) q0 vertex a(uint A[[vertex_id]],uint r[[instance_id]],constant TB&j[[buffer(R1(K4))]],constant g0*F[[buffer(0)]],const device p1*h0[[buffer(2)]],W8 Y0,U8 y3){q0 f0;
#define I6(a,n3,o3,C3,D3,p1,h0,A) __attribute__((visibility("default"))) q0 vertex a(uint A[[vertex_id]],uint r[[instance_id]],constant TB&j[[buffer(R1(K4))]],constant n3*o3[[buffer(0)]],constant C3*D3[[buffer(1)]],const device p1*h0[[buffer(2)]]){q0 f0;
#define C1(E5) f0.R0=E5;}return f0;
#define f3(z1,a) z1 __attribute__((visibility("default"))) fragment a(q0 f0[[stage_in]],F5 Y0){
#define v6(z1,a) z1 __attribute__((visibility("default"))) fragment a(q0 f0[[stage_in]],F5 Y0,bool w6[[front_facing]]){
#define M2(C) return C;}
#define L6 ,constant TB&j,c d0,F5 Y0,V8 y3,ab y6
#define Y2 ,j,d0,Y0,y3,y6
#define L3 ,F5 Y0
#define e1 ,Y0
#define f7
#define A5
#ifdef PLS_IMPL_DEVICE_BUFFER
#define M1 struct V1{
#ifdef PLS_IMPL_DEVICE_BUFFER_RASTER_ORDERED
#define z0(g,a) device uint*a[[buffer(R1(g+e6)),raster_order_group(0)]]
#define k1(g,a) device uint*a[[buffer(R1(g+e6)),raster_order_group(0)]]
#define H2(g,a) device atomic_uint*a[[buffer(R1(g+e6)),raster_order_group(0)]]
#else
#define z0(g,a) device uint*a[[buffer(R1(g+e6))]]
#define k1(g,a) device uint*a[[buffer(R1(g+e6))]]
#define H2(g,a) device atomic_uint*a[[buffer(R1(g+e6))]]
#endif
#define N1 };
#define W3 ,V1 V0,uint G0
#define Q1 ,V0,G0
#define K0(h) unpackUnorm4x8(V0.h[G0])
#define a1(h) V0.h[G0]
#define Z2(h) atomic_load_explicit(&V0.h[G0],memory_order::memory_order_relaxed)
#define A0(h,C) V0.h[G0]=packUnorm4x8(C)
#define d1(h,C) V0.h[G0]=(C)
#define a3(h,C) atomic_store_explicit(&V0.h[G0],C,memory_order::memory_order_relaxed)
#define y2(h)
#define h2(h)
#define e5(h,E) atomic_fetch_max_explicit(&V0.h[G0],E,memory_order::memory_order_relaxed)
#define f5(h,E) atomic_fetch_add_explicit(&V0.h[G0],E,memory_order::memory_order_relaxed)
#define z2
#define A2
#define X8(a) __attribute__((visibility("default"))) fragment a(V1 V0,constant TB&j[[buffer(R1(K4))]],q0 f0[[stage_in]],F5 Y0,ab y6,V8 y3){c d0=f0.R0.xy;N0 G=N0(metal::floor(d0));uint G0=G.y*j.n6+G.x;
#define P1(a) void X8(a)
#define d2 }
#define v2(a) i X8(a){i F1;
#define r3 }return F1;d2
#else
#define M1 struct V1{
#define z0(g,a) [[color(g)]]i a
#define k1(g,a) [[color(g)]]uint a
#define H2 k1
#define N1 };
#define W3 ,thread V1&G5,thread V1&V0
#define Q1 ,G5,V0
#define K0(h) G5.h
#define a1(h) G5.h
#define Z2(h) a1
#define A0(h,C) V0.h=(C)
#define d1(h,C) V0.h=(C)
#define a3(h) d1
#define y2(h) V0.h=G5.h
#define h2(h) V0.h=G5.h
e uint C5(thread uint&w0,uint x){uint X0=w0;w0=metal::max(X0,x);return X0;}
#define e5(h,E) C5(V0.h,E)
e uint D5(thread uint&w0,uint x){uint X0=w0;w0=X0+x;return X0;}
#define f5(h,E) D5(V0.h,E)
#define z2
#define A2
#define X8(a,...) V1 __attribute__((visibility("default"))) fragment a(__VA_ARGS__){c d0[[maybe_unused]]=f0.R0.xy;V1 V0;
#define P1(a,...) X8(a,V1 G5,constant TB&j[[buffer(R1(K4))]],q0 f0[[stage_in]],ab y6,F5 Y0,V8 y3)
#define d2 }return V0;
#define xh(a,...) struct vh{i wh[[k(0)]];V1 V0;};vh __attribute__((visibility("default"))) fragment a(__VA_ARGS__){c d0[[maybe_unused]]=f0.R0.xy;i F1;V1 V0;
#define v2(a) xh(a,V1 G5,constant TB&j[[buffer(R1(K4))]],q0 f0[[stage_in]],F5 Y0,V8 y3)
#define r3 }return{.wh=F1,.V0=V0};
#endif
#define y4 z0
#define discard discard_fragment()
using namespace metal;template<int T1>e vec<uint,T1>floatBitsToUint(vec<float,T1>x){return as_type<vec<uint,T1>>(x);}template<int T1>e vec<int,T1>floatBitsToInt(vec<float,T1>x){return as_type<vec<int,T1>>(x);}e uint floatBitsToUint(float x){return as_type<uint>(x);}e int floatBitsToInt(float x){return as_type<int>(x);}template<int T1>e vec<float,T1>uintBitsToFloat(vec<uint,T1>x){return as_type<vec<float,T1>>(x);}e float uintBitsToFloat(uint x){return as_type<float>(x);}e D unpackHalf2x16(uint x){return as_type<D>(x);}e uint packHalf2x16(D x){return as_type<uint>(x);}e i unpackUnorm4x8(uint x){return unpack_unorm4x8_to_half(x);}e uint packUnorm4x8(i x){return pack_half_to_unorm4x8(x);}e c unpackUnorm2x16(uint x){return unpack_unorm2x16_to_float(x);}e e0 inverse(e0 n1){e0 bb=e0(n1[1][1],-n1[0][1],-n1[1][0],n1[0][0]);float yh=(bb[0][0]*n1[0][0])+(bb[0][1]*n1[1][0]);return bb*(1/yh);}e v mix(v l,v b,o6 J1){v F7;for(int H0=0;H0<3;++H0) F7[H0]=J1[H0]?b[H0]:l[H0];return F7;}e c mix(c l,c b,H4 J1){c F7;for(int H0=0;H0<2;++H0) F7[H0]=J1[H0]?b[H0]:l[H0];return F7;}e c mix(c l,c b,float t){return mix(l,b,c(t));}e float mod(float x,float y){return fmod(x,y);}