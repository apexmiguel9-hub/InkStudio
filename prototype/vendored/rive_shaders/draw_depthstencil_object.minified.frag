#ifdef FRAGMENT
#ifdef DRAW_IMAGE_MESH
I3 e3(h5,a4,GC);
#ifdef ENABLE_ADVANCED_BLEND
p5(XD);
#endif
J3 i5 c4(W5) j5
#endif
f3(i,HB){
#ifdef DRAW_IMAGE_MESH
q(J5,c);q(K1,i);
#ifdef ENABLE_ADVANCED_BLEND
q(D1,N);
#endif
#else
q(X1,f);
#ifdef ENABLE_MODULATED_IMAGE
q(C2,S);
#endif
#ifdef FEATHER_ATLAS_BLIT
q(F2,c);
#endif
#ifdef ENABLE_ADVANCED_BLEND
q(g1,d);
#endif
#endif
#ifdef DRAW_IMAGE_MESH
i k=A7(GC,W5,J5,j.Ed)*K1;
#else
d o=
#ifdef FEATHER_ATLAS_BLIT
clamp(j2(ED,R9,F2,.0).x,J0(.0),J0(1.));
#else
1.;
#endif
i k=L7(
#ifdef ENABLE_MODULATED_IMAGE
C2,
#endif
#ifdef ENABLE_ADVANCED_BLEND
g3(g1),
#endif
X1 Y2);
#endif
#if defined(ENABLE_ADVANCED_BLEND)&&!defined(FIXED_FUNCTION_COLOR_OUTPUT)
#ifdef DRAW_IMAGE_MESH
k.xyz=F6(k);N p3=D1;
#else
N p3=g3(g1);
#endif
i O1=x6(XD);k.xyz=Y4(k.xyz,O1,p3)*k.w;
#endif
#ifndef DRAW_IMAGE_MESH
k*=o;
#endif
#ifdef NEEDS_GAMMA_CORRECTION
if(NEEDS_GAMMA_CORRECTION){k=q3(k);}
#endif
k.xyz=K2(k.xyz,k.w,d0.xy,j.F3,j.G3);M2(k);}
#endif
