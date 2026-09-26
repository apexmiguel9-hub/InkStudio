#ifdef VERTEX
void main(){gl_Position=vec4(mix(vec2(-1,1),vec2(1,-1),equal(gl_VertexID&ivec2(1,2),ivec2(0))),0,1);
#ifdef POST_INVERT_Y
gl_Position.y=-gl_Position.y;
#endif
}
#endif
#ifdef FRAGMENT
#extension GL_EXT_shader_pixel_local_storage:require
#ifdef GL_ARM_shader_framebuffer_fetch
#extension GL_ARM_shader_framebuffer_fetch:require
#else
#extension GL_EXT_shader_framebuffer_fetch:require
#endif
#ifdef CLEAR_COLOR
#if __VERSION__>=310
layout(binding=0,std140) uniform qj{uniform highp vec4 zh;}Ah;
#else
uniform mediump vec4 XE;
#endif
#endif
#ifdef GL_EXT_shader_pixel_local_storage
#ifdef STORE_COLOR
__pixel_local_inEXT V1
#else
__pixel_local_outEXT V1
#endif
{layout(rgba8) mediump vec4 m0;layout(r32ui) highp uint i0;layout(rgba8) mediump vec4 p4;layout(r32ui) highp uint G7;};
#ifndef GL_ARM_shader_framebuffer_fetch
#ifdef LOAD_COLOR
layout(location=0) inout mediump vec4 cb;
#endif
#endif
#ifdef STORE_COLOR
layout(location=0) out mediump vec4 cb;
#endif
void main(){
#ifdef CLEAR_COLOR
#if __VERSION__>=310
m0=Ah.zh;
#else
m0=XE;
#endif
#endif
#ifdef LOAD_COLOR
#ifdef GL_ARM_shader_framebuffer_fetch
m0=gl_LastFragColorARM;
#else
m0=cb;
#endif
#endif
#ifdef CLEAR_COVERAGE
G7=0u;
#endif
#ifdef CLEAR_CLIP
i0=0u;
#endif
#ifdef STORE_COLOR
cb=m0;
#endif
}
#else
layout(location=0) out mediump vec4 Bh;void main(){Bh=vec4(0,1,0,1);}
#endif
#endif
