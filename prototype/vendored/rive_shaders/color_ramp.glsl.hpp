#pragma once

#include "color_ramp.glsl.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char color_ramp[] = R"===(#ifdef CB
h1(g0)
#ifdef Z9
K(0,uint,TD);K(1,uint,UD);K(2,uint,VD);K(3,uint,WD);
#else
K(0,R,IC);
#endif
i1
#endif
q2 I0 W(0,i,V6);i2
#ifdef CB
Y3 Z3 F4 G4 i Ff(uint k){return sc((R(k,k,k,k)>>R(16,8,0,24))&0xffu)/255.;}B1(LF,g0,F,A,r){
#ifdef Z9
L(r,F,TD,uint);L(r,F,UD,uint);L(r,F,VD,uint);L(r,F,WD,uint);R IC=R(TD,UD,VD,WD);
#else
L(r,F,IC,R);
#endif
V(V6,i);int n8=A>>1;float x=float(n8<=1?IC.x&0xffffu:IC.x>>16)/65536.;float aa=(A&1)==0?.0:1.;if(j.tc<.0){aa=1.-aa;}uint W6=IC.y;float y=float(W6&~Gf)+aa;if((W6&uc)!=0u&&n8==0){if((W6&ba)!=0u) x=.0;else x-=vc;}if((W6&wc)!=0u&&n8==3){if((W6&ba)!=0u) x=1.;else x+=vc;}V6=Ff(n8<=1?IC.z:IC.w);f X=o8(c(x,y),2.,j.tc);
#ifdef RC
X.y=-X.y;
#endif
c0(V6);C1(X);}
#endif
#ifdef EB
I3 J3 f3(i,MF){q(V6,i);M2(V6);}
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive