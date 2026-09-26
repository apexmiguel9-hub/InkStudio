#pragma once

#include "tessellate.glsl.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char tessellate[] = R"===(#define Yh 10
#ifdef CB
h1(g0) K(0,f,LD);K(1,f,MD);K(2,f,UC);
#ifdef Z9
K(3,uint,HE);K(4,uint,IE);K(5,uint,JE);K(6,uint,KE);
#else
K(3,R,UB);
#endif
i1
#endif
q2 I0 W(0,f,A6);I0 W(1,f,B6);I0 W(2,f,R4);I0 W(3,S,S4);V2 W(4,uint,H7);i2
#ifdef CB
Y3 h6(h3,h7,XC);Z3 g4(h7,ca) F4 M4(Yc,Hg,OB);M4(Zc,Ig,HD);G4 B1(EG,g0,F,A,r){L(r,F,LD,f);L(r,F,MD,f);L(r,F,UC,f);
#ifdef Z9
L(r,F,HE,uint);L(r,F,IE,uint);L(r,F,JE,uint);L(r,F,KE,uint);R UB=R(HE,IE,JE,KE);
#else
L(r,F,UB,R);
#endif
V(A6,f);V(B6,f);V(R4,f);V(S4,S);V(H7,uint);c x0=LD.xy;c B0=LD.zw;c F0=MD.xy;c M0=MD.zw;bool ge=A<4;float y=ge?UC.z:UC.w;int ib=int(ge?UB.x:UB.y);
#ifdef da
int he=ib<<16;if(UB.z==0xffffffffu){--he;}float a9=float(he>>16);
#else
float a9=float(ib<<16>>16);
#endif
float c9=float(ib>>16);c r2=c((A&1)==0?a9:c9,(A&2)==0?y+1.:y);if((c9-a9)*j.Cd<.0){r2.y=2.*y+1.-r2.y;}uint U2=UB.z&0x3ffu;uint ie=(UB.z>>10)&0x3ffu;uint n2=UB.z>>20;uint j0=UB.w;uint F8=j0&Uc;uint o0=F8>0u?L0(HD,max(F8,1u)-1u).z:0u;R O4=o0!=0u?L0(OB,o0*4u+1u):R(0u,0u,0u,0u);float N2=uintBitsToFloat(O4.z);float O2=uintBitsToFloat(O4.w);if(O2!=.0&&N2==.0){float je;float Zh=Cf(x0,B0,F0,M0,je);float jb=O2*(1./qa);float ai=xf(x0,B0,F0,M0,je,jb);float I7=1.-ai*(1./H3);float bi=dot(M0-x0,M0-x0)/(jb*jb);float ci=(bi-1.)*.5;I7=min(I7,ci);I7=min(I7,.99);float di=.5*I7;float x=zc(di)*-2.+1.;float ke=k8(x*O2,Zh);f le=mix(x0.xyxy,M0.xyxy,f(1./3.,1./3.,2./3.,2./3.));B0=mix(B0,le.xy,ke);F0=mix(F0,le.zw,ke);}if((j0&dg)!=0u){e0 T8=L1(uintBitsToFloat(L0(OB,o0*4u)));c me=P0(T8,-2.*B0+F0+x0);c ne=P0(T8,-2.*F0+M0+B0);float n1=max(dot(me,me),dot(ne,ne));float T3=max(ceil(sqrt(.75*4.*sqrt(n1))),1.);U2=min(uint(T3),U2);}uint d9=U2+ie+n2-1u;e0 L2=W9(x0,B0,F0,M0);float f1=acos(V9(L2[0],L2[1]));float r4=f1/float(ie);float kb=determinant(e0(F0-x0,M0-B0));if(kb==.0) kb=determinant(L2);if(kb<.0) r4=-r4;A6=f(x0,B0);B6=f(F0,M0);R4=f(float(d9)-abs(c9-r2.x),float(d9),(n2<<10)|U2,r4);S4.xy=UC.xy;if(n2>1u){e0 lb=e0(L2[1],UC.xy);float ei=acos(V9(lb[0],lb[1]));float oe=float(n2);if((j0&(f4|z8))==(x8|z8)){oe-=2.;}float mb=ei/oe;if(determinant(lb)<.0) mb=-mb;S4.z=mb;}if(c9<a9){j0|=K3;}H7=j0;f X=o8(r2,2./ag,j.Cd);
#ifdef RC
X.y=-X.y;
#endif
c0(A6);c0(B6);c0(R4);c0(S4);c0(H7);C1(X);}
#endif
#ifdef EB
I3 J3 f3(R,FG){q(A6,f);q(B6,f);q(R4,f);q(S4,S);q(H7,uint);c x0=A6.xy;c B0=A6.zw;c F0=B6.xy;c M0=B6.zw;e0 L2=W9(x0,B0,F0,M0);float fi=max(floor(R4.x),.0);float d9=R4.y;uint pe=uint(R4.z);float U2=float(pe&0x3ffu);float n2=float(pe>>10);float r4=R4.w;uint j0=H7;float T4=d9-n2;float W1=fi;if(W1<=T4){j0&=~f4;}else{x0=B0=F0=M0;L2=e0(L2[1],S4.xy);U2=1.;W1-=T4;T4=n2;r4=S4.z;if((j0&f4)>x8){if(W1<2.5) j0|=ra;if(W1>1.5&&W1<3.5) j0|=Sc;}else if((j0&z8)!=0u||(j0&f4)==y8){T4-=2.;--W1;}j0|=r4<.0?A8:Tc;}c H5;float f1=.0;if(W1==.0||W1==T4||(j0&f4)>x8){bool I8=W1<T4*.5;H5=I8?x0:M0;f1=Bc(I8?L2[0]:L2[1]);}else if((j0&Rc)!=0u){H5=x0;if(W1>=float(oa/2u)) H5=B0;if(W1>=float(oa*3u/4u)) H5=F0;if(W1>=float(oa*7u/8u)) H5=S4.xy;}else{float w1,I5;if(U2==T4){w1=W1/U2;I5=.0;}else{c B,H,l2=B0-x0;c O6=M0-x0;c h8=F0-B0;H=h8-l2;B=-3.*h8+O6;c gi=H*(U2*2.);c Q6=l2*(U2*U2);float e9=.0;float hi=min(U2-1.,W1);c nb=normalize(L2[0]);float ii=-abs(r4);float ji=(1.+W1)*abs(r4);for(int ob=Yh-1;ob>=0;--ob){float J7=e9+exp2(float(ob));if(J7<=hi){c pb=J7*B+gi;pb=J7*pb+Q6;float ki=dot(normalize(pb),nb);float qb=J7*ii+ji;qb=min(qb,H3);if(ki>=cos(qb)) e9=J7;}}float li=e9/U2;float qe=W1-e9;float f9=acos(clamp(nb.x,-1.,1.));f9=nb.y>=.0?f9:-f9;f1=qe*r4+f9;c d3=c(sin(f1),-cos(f1));float l=dot(d3,B),g9=dot(d3,H),J1=dot(d3,l2);float mi=max(g9*g9-l*J1,.0);float x2=sqrt(mi);if(g9>.0) x2=-x2;x2-=g9;float re=-.5*x2*l;c rb=(abs(x2*x2+re)<abs(l*J1+re))?c(x2,l):c(J1,x2);I5=(rb.y!=.0)?rb.x/rb.y:.0;I5=clamp(I5,.0,1.);if(qe==.0) I5=.0;w1=max(li,I5);}c ni=a6(x0,B0,w1);c se=a6(B0,F0,w1);c oi=a6(F0,M0,w1);c te=a6(ni,se,w1);c ue=a6(se,oi,w1);H5=a6(te,ue,w1);if(w1!=I5) f1=Bc(ue-te);}R K7;K7.xy=floatBitsToUint(H5);if((j0&f4)==y8){K7.z=(uint(T4)<<16)|uint(W1);}else{K7.z=floatBitsToUint(mod(f1,p8));}K7.w=j0;M2(K7);}
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive