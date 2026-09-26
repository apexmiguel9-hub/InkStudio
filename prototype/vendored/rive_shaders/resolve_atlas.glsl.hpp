#pragma once

#include "resolve_atlas.glsl.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char resolve_atlas[] = R"===(#ifdef CB
B1(YF,g0,F,A,r){f X;X.x=(A!=2)?-1.:3.;X.y=(A!=1)?-1.:3.;X.zw=c(.0,1.);C1(X);}
#endif
#ifdef EB
e ivec2 ee(){return ivec2(floor(gl_FragCoord));}
#ifdef YD
layout(location=0) inout R r0;layout(location=1) out i q4;void main(){q4.x=uintBitsToFloat(r0.x);}
#elif defined(ZD)
#ifdef FE
__pixel_local_outEXT V1{layout(r32f) float r0;};
#else
__pixel_local_inEXT V1{layout(r32f) float r0;};layout(location=0) out i q4;
#endif
void main(){
#ifdef FE
r0=.0;
#else
q4.x=r0;
#endif
}
#elif defined(EXPORTED_ATLAS_RENDER_TARGET_R32UI_PLS_ANGLE)
layout(binding=0,r32ui) uniform highp upixelLocalANGLE r0;layout(location=0) out i q4;void main(){q4.x=uintBitsToFloat(pixelLocalLoadANGLE(r0).x);}
#elif defined(AE)
layout(binding=0,r32i) uniform highp coherent iimage2D Y8;layout(location=0) out i q4;void main(){q4.x=float(imageLoad(Y8,ee()).x)*(1./dd);}
#elif defined(ZE)
e3(h3,0,CF);layout(location=0) out i q4;void main(){i P=v1(CF,ee());q4.x=(P.x-P.y)*xa+(P.z-P.w)*255.;}
#endif
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive