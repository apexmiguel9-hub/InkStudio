#ifdef FRAGMENT
M1
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
z0(G2,m0);
#endif
k1(X2,i0);
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
Ya(f6,D6);
#endif
k1(J6,S0);N1
#ifdef FIXED_FUNCTION_COLOR_OUTPUT
v2(HB)
#else
P1(HB)
#endif
{q(X1,f);
#ifdef ENABLE_MODULATED_IMAGE
q(C2,S);
#endif
#ifdef DRAW_INTERIOR_TRIANGLES
q(j1,d);
#else
q(O,B2);
#endif
q(D0,d);
#ifdef ENABLE_CLIPPING
q(Y1,D);
#endif
#ifdef ENABLE_CLIP_RECT
q(O0,f);
#endif
#ifdef ENABLE_ADVANCED_BLEND
q(g1,d);
#endif
d y0=
#ifdef DRAW_INTERIOR_TRIANGLES
j1;
#else
tb(O);
#endif
i k0;d I1;
#if defined(DRAW_INTERIOR_TRIANGLES)&&defined(BORROWED_COVERAGE_PASS)
if(!BORROWED_COVERAGE_PASS)
#endif
{k0=L7(
#ifdef ENABLE_MODULATED_IMAGE
C2,
#endif
#ifdef ENABLE_ADVANCED_BLEND
g3(g1),
#endif
X1 Y2);I1=1.;
#ifdef ENABLE_CLIP_RECT
if(ENABLE_CLIP_RECT){d yb=m3(g5(O0));I1=min(yb,I1);}
#endif
}z2;
#if defined(DRAW_INTERIOR_TRIANGLES)&&defined(BORROWED_COVERAGE_PASS)
if(BORROWED_COVERAGE_PASS){d1(S0,packHalf2x16(D2(y0,D0)));
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
y2(m0);
#endif
}else
#endif
{D U4=unpackHalf2x16(a1(S0));d j9=U4.y;d W4=j9==D0?U4.x:J0(.0);d Ge=
#ifndef DRAW_INTERIOR_TRIANGLES
V5(O)?max(W4,y0):
#endif
W4+y0;
#ifdef ENABLE_CLIPPING
if(ENABLE_CLIPPING&&Y1.x!=.0){D Q0=unpackHalf2x16(a1(i0));d M5=Q0.y;d zb=M5==Y1.x?Q0.x:J0(.0);I1=min(zb,I1);}
#endif
I1=max(I1,.0);d e2=ga(W4,.0,I1);d H1=ga(Ge,.0,I1);
#ifdef ENABLE_DITHER
d L5;if(ENABLE_DITHER){L5=ja(d0.xy,j.F3,j.G3);}
#endif
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
i O1=K0(m0);
#ifdef ENABLE_ADVANCED_BLEND
if(ENABLE_ADVANCED_BLEND&&g1!=Z5(B4)){if(H1!=.0){if(e2==.0){k0.xyz=Y4(k0.xyz,O1,g3(g1));
#ifndef DRAW_INTERIOR_TRIANGLES
if(H1<I1){v P7=k0.xyz;
#ifdef ENABLE_DITHER
if(ENABLE_DITHER){P7+=L5*j.Fd;}
#endif
A0(D6,E0(P7,0.0));}
#endif
}else{k0.xyz=K0(D6).xyz;y2(D6);}}k0.xyz*=k0.w;}
#endif
#endif
k0*=K8(e2,H1,k0.w);
#ifdef ENABLE_DITHER
k0.xyz=K2(k0.xyz,k0.w,L5);
#endif
#ifndef DRAW_INTERIOR_TRIANGLES
#ifdef ENABLE_ADVANCED_BLEND
#define He (!ENABLE_ADVANCED_BLEND||g1==Z5(B4))&&k0.w>=1.
#else
#define He k0.w>=1.
#endif
Vd(He,S0,packHalf2x16(D2(Ge,D0)));
#else
h2(S0);
#endif
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
Ud(k0.x+k0.y+k0.z+k0.w==.0,m0,O1*(1.-k0.w)+k0);
#endif
}h2(i0);A2;
#ifdef FIXED_FUNCTION_COLOR_OUTPUT
F1=k0;r3
#else
d2;
#endif
}
#endif
