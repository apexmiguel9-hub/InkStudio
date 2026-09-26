#define da
#ifndef GLSL_VERSION
#define GLSL_VERSION __VERSION__
#endif
#define c vec2
#define S vec3
#define R3 vec3
#define f vec4
#define d mediump float
#define D mediump vec2
#define v mediump vec3
#define i mediump vec4
#define Z6 mediump mat3x3
#define a7 mediump mat2x3
#define I4 mediump mat4x4
#define Y ivec2
#define d6 ivec4
#define N0 uvec2
#define R uvec4
#define N mediump uint
#define H4 bvec2
#define o6 bvec3
#define y7 bvec4
#define e0 mat2
#define e
#define c1(o2) out o2
#define K6(o2) inout o2
#ifdef GL_ANGLE_base_vertex_base_instance_shader_builtin
#extension GL_ANGLE_base_vertex_base_instance_shader_builtin:require
#endif
#ifdef ENABLE_KHR_BLEND
#extension GL_KHR_blend_equation_advanced:require
#endif
#ifdef ATLAS_RENDER_TARGET_R32UI_FRAMEBUFFER_FETCH
#extension GL_EXT_shader_framebuffer_fetch:require
#elif defined(ATLAS_RENDER_TARGET_R8_PLS_EXT)
#extension GL_EXT_shader_pixel_local_storage:require
#elif defined(ATLAS_RENDER_TARGET_R32UI_PLS_ANGLE)
#extension GL_ANGLE_shader_pixel_local_storage:require
#elif defined(ATLAS_RENDER_TARGET_R32I_ATOMIC_TEXTURE)
#ifdef GL_ARB_shader_image_load_store
#extension GL_ARB_shader_image_load_store:require
#endif
#ifdef GL_OES_shader_image_atomic
#extension GL_OES_shader_image_atomic:require
#endif
#endif
#if defined(RENDER_MODE_DEPTH_STENCIL)&&defined(ENABLE_CLIP_RECT)&&defined(GL_ES)&&!defined(DISABLE_CLIP_DISTANCE_FOR_UBERSHADERS)
#ifdef GL_EXT_clip_cull_distance
#extension GL_EXT_clip_cull_distance:require
#elif defined(GL_ANGLE_clip_cull_distance)
#extension GL_ANGLE_clip_cull_distance:require
#endif
#endif
#if GLSL_VERSION>=310
#define x7(g,a) layout(binding=g,std140) uniform a{
#else
#define x7(g,a) layout(std140) uniform a{
#endif
#define M8(a) }a;
#define Ld(a) layout(push_constant) uniform a{
#define Md(Z,a) Z a;
#define Nd(a) }a;
#define h1(a)
#define K(g,Z,a) layout(location=g) in Z a
#define i1
#define L(P8,F,a,Z)
#ifdef VERTEX
#if GLSL_VERSION>=310
#define W(g,Z,a) layout(location=g) out Z a
#else
#define W(g,Z,a) out Z a
#endif
#else
#if GLSL_VERSION>=310
#define W(g,Z,a) layout(location=g) in Z a
#else
#define W(g,Z,a) in Z a
#endif
#endif
#define V2 flat
#define q2
#define i2
#ifdef TARGET_SPIRV
#define I0
#else
#ifdef GL_NV_shader_noperspective_interpolation
#extension GL_NV_shader_noperspective_interpolation:require
#define I0 noperspective
#else
#define I0
#endif
#endif
#ifdef VERTEX
#define Y3
#define Z3
#endif
#ifdef FRAGMENT
#define I3
#define J3
#endif
#define i5
#define j5
#ifdef TARGET_SPIRV
#define L4(a0,g,a) layout(set=a0,binding=g) uniform highp utexture2D a
#define p6(a0,g,a) layout(set=a0,binding=g) uniform highp texture2D a
#define e3(a0,g,a) layout(set=a0,binding=g) uniform mediump texture2D a
#define o5(a0,g,a) layout(binding=g) uniform mediump texture2D a
#if defined(FRAGMENT)&&defined(RENDER_MODE_DEPTH_STENCIL)
#endif
#elif GLSL_VERSION>=310
#define L4(a0,g,a) layout(binding=g) uniform highp usampler2D a
#define p6(a0,g,a) layout(binding=g) uniform highp sampler2D a
#define e3(a0,g,a) layout(binding=g) uniform mediump sampler2D a
#define o5(a0,g,a) layout(binding=g) uniform mediump sampler2D a
#else
#define L4(a0,g,a) uniform highp usampler2D a
#define p6(a0,g,a) uniform highp sampler2D a
#define e3(a0,g,a) uniform mediump sampler2D a
#define o5(a0,g,a) uniform mediump sampler2D a
#endif
#ifdef TARGET_SPIRV
#define q6(a0,g,a) layout(set=a0,binding=g) uniform mediump sampler a;
#ifdef USE_WEBGPU_SAMPLERS
#define g4(z7,a) layout(set=mg,binding=z7) uniform mediump sampler a;
#define c4(a) q6(h5,lg,a)
#else
#define g4(z7,a) layout(set=h3,binding=z7) uniform mediump sampler a;
#define c4(a) q6(h5,a4,a)
#endif
#define y5(a,p,m) texture(sampler2D(a,p),m)
#define j2(a,p,m,U0) textureLod(sampler2D(a,p),m,U0)
#define z5(a,p,m,U1) texture(sampler2D(a,p),m,U1)
#if defined(FRAGMENT)&&defined(RENDER_MODE_DEPTH_STENCIL)&&defined(MSAA_DST_COLOR)
#extension GL_OES_sample_variables:require
#endif
#else
#define g4(z7,a)
#define q6(a0,g,a)
#define c4(a)
#define y5(a,p,m) texture(a,m)
#define j2(a,p,m,U0) textureLod(a,m,U0)
#define z5(a,p,m,U1) texture(a,m,U1)
#endif
#define g8(n0,p,m) y5(n0,p,m)
#define U6(n0,p,m,U0) j2(n0,p,m,U0)
#define A7(n0,p,m,U1) z5(n0,p,m,U1)
#define h6(a0,g,a) o5(a0,g,a)
#define Y6(a,p,E,r6,R8,U0) j2(a,p,c(E,R8),U0)
#define mh(a0,g,a) L4(a0,g,a)
#define L3
#define e1
#define v1(a,m) texelFetch(a,m,0)
#ifdef TARGET_SPIRV
#elif GLSL_VERSION>=310
#else
#endif
#define F4
#define G4
#define U3
#define V3
#ifdef DISABLE_SHADER_STORAGE_BUFFERS
#define O5(g,y1,a) L4(h3,g,a)
#define M4(g,y1,a) mh(h3,g,a)
#define P5(g,y1,a) p6(h3,g,a)
#define L0(a,C0) v1(a,Y((C0)&Qc,(C0)>>Pc))
#define R5(a,C0) v1(a,Y((C0)&Qc,(C0)>>Pc)).xy
#else
#ifdef GL_ARB_shader_storage_buffer_object
#extension GL_ARB_shader_storage_buffer_object:require
#endif
#define O5(g,y1,a) layout(std430,binding=g) readonly buffer y1{N0 g2[];}a
#define M4(g,y1,a) layout(std430,binding=g) readonly buffer y1{R g2[];}a
#define P5(g,y1,a) layout(std430,binding=g) readonly buffer y1{f g2[];}a
#define Sa(g,y1,a) layout(std430,binding=g) buffer y1{uint g2[];}a
#define L0(a,C0) a.g2[C0]
#define R5(a,C0) a.g2[C0]
#define Rd(a,C0) a.g2[C0]
#define C7(a,C0,E) atomicMax(a.g2[C0],E)
#define Ta(a,C0,E) atomicAdd(a.g2[C0],E)
#define nh(a,C0,E) atomicOr(a.g2[C0],E)
#endif
#ifdef PLS_IMPL_STORAGE_BUFFER
#define P1(a) void main(){Y G=ivec2(floor(d0));int G0=int(L8(uvec2(G),(j.n6+(ya-1u))&~(ya-1u)));
#define d2 }
#define W3 ,int G0
#define Q1 ,G0
#ifdef TARGET_WGSL
#define H2(g,a) layout(std430,set=w3,binding=g) buffer a##Sd{uint g2[];}a
#elif defined(TARGET_SPIRV)
#define H2(g,a) layout(std430,set=w3,binding=g) coherent buffer a##Sd{uint g2[];}a
#else
#define H2(g,a) layout(std430,binding=g) coherent buffer a##Sd{uint g2[];}a
#endif
#define Ua H2
#define Z2(h) h.g2[G0]
#define a3(h,C) h.g2[G0]=C
#define Va(h) unpackUnorm4x8(Z2(h))
#define Wa(h,C) a3(h,packUnorm4x8(C))
#define e5(h,E) atomicMax(h.g2[G0],E)
#define f5(h,E) atomicAdd(h.g2[G0],E)
#elif defined(PLS_IMPL_STORAGE_TEXTURE)||defined(USING_PLS_STORAGE_TEXTURES)
#ifdef GL_ARB_shader_image_load_store
#extension GL_ARB_shader_image_load_store:require
#endif
#define P1(a) void main(){Y G=ivec2(floor(d0));
#define d2 }
#define W3 ,Y G
#define Q1 ,G
#ifdef TARGET_SPIRV
#define Ua(g,a) layout(set=w3,binding=g,rgba8) uniform mediump coherent image2D a
#define H2(g,a) layout(set=w3,binding=g,r32ui) uniform highp coherent uimage2D a
#define Xa(g,a) layout(set=w3,binding=g,rgb10_a2) uniform mediump coherent image2D a
#else
#define Ua(g,a) layout(binding=g,rgba8) uniform mediump coherent image2D a
#define H2(g,a) layout(binding=g,r32ui) uniform highp coherent uimage2D a
#define Xa(g,a) layout(binding=g,rgb10_a2) uniform mediump coherent image2D a;
#endif
#define Z2(h) imageLoad(h,G).x
#define a3(h,C) imageStore(h,G,uvec4(C))
#define Va(h) imageLoad(h,G)
#define Wa(h,C) imageStore(h,G,C)
#define e5(h,E) imageAtomicMax(h,G,E)
#define f5(h,E) imageAtomicAdd(h,G,E)
#else
#define P1(a) void main()
#define d2
#define W3
#define Q1
#endif
#ifdef PLS_IMPL_ANGLE
#extension GL_ANGLE_shader_pixel_local_storage:require
#define M1
#define z0(g,a) layout(binding=g,rgba8) uniform mediump pixelLocalANGLE a
#define k1(g,a) layout(binding=g,r32ui) uniform highp upixelLocalANGLE a
#define N1
#define K0(h) pixelLocalLoadANGLE(h)
#define a1(h) pixelLocalLoadANGLE(h).x
#define A0(h,C) pixelLocalStoreANGLE(h,C)
#define d1(h,C) pixelLocalStoreANGLE(h,uvec4(C))
#define y2(h)
#define h2(h)
#define z2
#define A2
#endif
#ifdef PLS_IMPL_EXT_NATIVE
#ifdef FIXED_FUNCTION_COLOR_OUTPUT
#extension GL_EXT_shader_pixel_local_storage2:require
#else
#extension GL_EXT_shader_pixel_local_storage:require
#endif
#define M1 __pixel_localEXT V1{
#define z0(g,a) layout(rgba8) mediump vec4 a
#define Ya(g,a) layout(rgb10_a2) mediump vec4 a
#define k1(g,a) layout(r32ui) highp uint a
#define N1 };
#define K0(h) h
#define a1(h) h
#define A0(h,C) h=(C)
#define d1(h,C) h=(C)
#define y2(h) h=h
#define h2(h) h=h
#define z2
#define A2
#ifdef FIXED_FUNCTION_COLOR_OUTPUT
#define v2(a) layout(location=0,rgba8) out i F1;P1(a)
#endif
#endif
#if defined(PLS_IMPL_STORAGE_TEXTURE)||defined(PLS_IMPL_STORAGE_BUFFER)
#define M1
#define N1
#define z0 Ua
#define k1 H2
#define Ya Xa
#define K0 Va
#define A0 Wa
#define a1 Z2
#define d1 a3
#define y2(h)
#define h2(h)
#if defined(GL_ARB_fragment_shader_interlock)
#extension GL_ARB_fragment_shader_interlock:require
#define z2 beginInvocationInterlockARB()
#define A2 endInvocationInterlockARB()
#elif defined(GL_INTEL_fragment_shader_ordering)
#extension GL_INTEL_fragment_shader_ordering:require
#define z2 beginFragmentShaderOrderingINTEL()
#define A2
#else
#define z2
#define A2
#endif
#endif
#ifdef PLS_IMPL_SUBPASS_LOAD
#define M1
#define y4(g,a) layout(input_attachment_index=g,binding=g,set=w3) uniform mediump subpassInput D7##a
#define Td(g,a) layout(location=g) out mediump vec4 a
#define z0(g,a) y4(g,a);Td(g,a)
#define k1(g,a) layout(input_attachment_index=g,binding=g,set=w3) uniform highp usubpassInput D7##a;layout(location=g) out highp uvec4 a
#define N1
#define K0(h) subpassLoad(D7##h)
#define a1(h) subpassLoad(D7##h).x
#define A0(h,C) h=(C)
#define d1(h,C) h.x=(C)
#define y2(h) A0(h,subpassLoad(D7##h))
#define h2(h) d1(h,subpassLoad(D7##h).x)
#define z2
#define A2
#endif
#ifdef PLS_IMPL_NONE
#define M1
#define z0(g,a) layout(location=g) out mediump vec4 a
#define k1(g,a) layout(location=g) out highp uvec4 a
#define N1
#define K0(h) vec4(0)
#define a1(h) 0u
#define A0(h,C) h=(C)
#define d1(h,C) h.x=(C)
#define y2(h) h=vec4(0)
#define h2(h) h.x=0u
#define z2
#define A2
#endif
#ifndef y4
#define y4 z0
#endif
#ifdef TARGET_SPIRV
#define gl_VertexID gl_VertexIndex
#endif
#ifdef ENABLE_INSTANCE_INDEX
#ifdef TARGET_SPIRV
#define S8 gl_InstanceIndex
#else
#ifdef BASE_INSTANCE_UNIFORM_NAME
uniform highp int BASE_INSTANCE_UNIFORM_NAME;
#define S8 (gl_InstanceID+BASE_INSTANCE_UNIFORM_NAME)
#else
#define S8 (gl_InstanceID+gl_BaseInstance)
#endif
#endif
#else
#define S8 0
#endif
#define k6
#define A3
#define f7
#define A5
#define B1(a,g0,F,A,r) void main(){int A=gl_VertexID;int r=S8;
#define S7(a,g0,F,p1,h0,A,r) B1(a,g0,F,A,r)
#define I6(a,n3,o3,C3,D3,p1,h0,A) B1(a,n3,o3,A,r)
#define V(a,Z)
#define c0(a)
#define q(a,Z)
#define C1(R0) gl_Position=R0;}
#define f3(z1,a) layout(location=0) out z1 oh;void main()
#define v6(z1,a) f3(z1,a)
#define w6 gl_FrontFacing
#define M2(C) oh=C
#define d0 gl_FragCoord.xy
#define L6
#define Y2
#if defined(PLS_IMPL_STORAGE_TEXTURE)||defined(PLS_IMPL_STORAGE_BUFFER)
#define Ud(E7,h,C) if(!(E7)){A0(h,C);}
#define Vd(E7,h,C) if(!(E7)){d1(h,C);}
#else
#define Ud(E7,h,C) A0(h,C);
#define Vd(E7,h,C) d1(h,C);
#endif
#ifndef v2
#define v2(a) layout(location=0) out i F1;P1(a)
#endif
#define r3 d2
#if defined(TARGET_SPIRV)&&!defined(TARGET_WGSL)
#ifdef MSAA_DST_COLOR
#define p5(a) layout(input_attachment_index=0,binding=G2,set=w3) uniform mediump subpassInputMS a
#define x6(a) na(mat4(subpassLoad(a,0),subpassLoad(a,1),subpassLoad(a,2),subpassLoad(a,3)),gl_SampleMaskIn[0])
#else
#define p5(a) layout(input_attachment_index=0,binding=G2,set=w3) uniform mediump subpassInput a
#define x6(a) subpassLoad(a)
#endif
#else
#define p5(a) e3(h3,kg,a)
#define x6(a) texelFetch(a,ivec2(floor(d0.xy)),0)
#endif
#define P0(B,H) ((B)*(H))
precision highp float;precision highp int;
#if GLSL_VERSION<310
e i ph(uint u){R l1=R(u&0xffu,(u>>8)&0xffu,(u>>16)&0xffu,u>>24);return f(l1)*(1./255.);}
#define unpackUnorm4x8 ph
#endif
