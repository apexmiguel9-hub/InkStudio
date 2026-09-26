#pragma once

#include "bezier_utils.glsl.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char bezier_utils[] = R"===(#ifndef gc
#define gc f
#endif
#ifndef N6
#define N6 c
#endif
e float V9(c l,c b){float vf=dot(l,b);float hc=dot(l,l)*dot(b,b);return(hc==.0)?1.:clamp(vf*inversesqrt(hc),-1.,1.);}e void wf(c x0,c B0,c F0,c M0,c1(c) B,c1(c) H,c1(c) l2){l2=B0-x0;c O6=F0-B0;c h8=M0-x0;H=O6-l2;B=-3.*O6+h8;}e e0 W9(c x0,c B0,c F0,c M0){e0 t;t[0]=(any(notEqual(x0,B0))?B0:any(notEqual(B0,F0))?F0:M0)-x0;t[1]=M0-(any(notEqual(M0,F0))?F0:any(notEqual(F0,B0))?B0:x0);return t;}e float xf(c x0,c B0,c F0,c M0,float w1,float yf){c B,H,l2;wf(x0,B0,F0,M0,B,H,l2);c P6=3.*(((B*w1)+2.*H)*w1+l2);float ic=length(P6);if(ic==.0){return.0;}P6*=1./ic;float i8=2.*dot(B,P6);float Q6=3.*(i8*w1+4.*dot(H,P6))*w1+6.*dot(l2,P6);float X9=min(w1,1.-w1);float zf=(i8*X9*X9+Q6)*X9;float jc=min(yf,zf*.9999);float c3;if(i8==.0){c3=jc/Q6;}else{float M=1./i8;float b=Q6*M,J1=-jc*M;float R6=(-1./3.)*b,S6=.5*J1;float kc=S6*S6-R6*R6*R6;if(kc<.0){float j8=sqrt(R6);float f1=acos(S6/(j8*j8*j8));c3=-2.*j8*cos(f1*(1./3.)+(-H3*2./3.));}else{float B=pow(abs(S6)+sqrt(kc),1./3.);if(S6<.0) B=-B;c3=B!=.0?B+R6/B:.0;}}c3=abs(c3);f t0011=w1+gc(-c3,-c3,c3,c3);f lc=(B.xyxy*t0011+2.*H.xyxy)*t0011+l2.xyxy;e0 L2=W9(x0,B0,F0,M0);c Af=t0011.x<1e-3?L2[0]:lc.xy;c Bf=t0011.z>1.-1e-3?L2[1]:lc.zw;return acos(V9(Af,Bf));}e float k8(float l,float b){l=b<.0?-l:l;b=abs(b);return l>.0?(l<b?l/b:1.):.0;}float Cf(c x0,c B0,c F0,c M0,c1(float) Y9){c mc=M0-x0;float nc=length(M0-x0);if(nc==.0){Y9=.5;return.0;}c d3=N6(-mc.y,mc.x)/nc;float oc=dot(d3,F0-x0);float D4=dot(d3,B0-x0);float E4=D4-oc;
#if 0
float l=3.*E4;float pc=E4+D4;float J1=D4;float x2=sqrt(max(E4*E4+oc*D4,.0));if(pc<.0) x2=-x2;x2+=pc;c T6=N6(k8(x2,l),k8(J1,x2));c Y5=3.*(T6*(T6*(T6*E4-(D4+E4))+D4));Y5=abs(Y5);Y9=Y5.x>Y5.y?T6.x:T6.y;return max(Y5.x,Y5.y);
#else
float qc=3.*E4;float H=-D4-E4;float l2=D4;float t=.5;for(int H0=0;H0<3;++H0){float rc=qc*t;t=k8(rc*t-l2,2.*(rc+H));}Y9=t;return abs(t*(t*(t*qc+3.*H)+3.*l2));
#endif
}
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive