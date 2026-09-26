#ifdef VERTEX
h1(g0) K(0,f,VB);K(1,f,WB);i1
#endif
q2 I0 W(0,f,O);i2
#ifdef VERTEX
B1(XF,g0,F,A,r){L(A,F,VB,f);L(A,F,WB,f);V(O,f);f X;uint o0;c l0;if(r9(VB,WB,r,o0,l0,O A3)){R Q4=L0(OB,o0*4u+2u);S v7=uintBitsToFloat(Q4.yzw);l0=l0*v7.x+v7.yz;X=o8(l0,j.Dd.x,j.Dd.y);
#ifdef POST_INVERT_Y
X.y=-X.y;
#endif
}else{X=f(j.W2,j.W2,j.W2,j.W2);}c0(O);C1(X);}
#endif
#ifdef FRAGMENT
#ifdef ATLAS_FEATHERED_FILL
e d z6(f P,bool Ch L3){d o=d8(P e1);if(!Ch) o=-o;return o;}
#endif
#ifdef ATLAS_RENDER_TARGET_R32UI_FRAMEBUFFER_FETCH
layout(location=0) inout R r0;
#ifdef ATLAS_FEATHERED_FILL
void main(){float o=uintBitsToFloat(r0.x);o+=z6(O,gl_FrontFacing e1);r0.x=floatBitsToUint(o);}
#endif
#ifdef ATLAS_FEATHERED_STROKE
void main(){float o=uintBitsToFloat(r0.x);o=max(o,C4(O));r0.x=floatBitsToUint(o);}
#endif
#elif defined(ATLAS_RENDER_TARGET_R8_PLS_EXT)
__pixel_localEXT V1{layout(r32f) float r0;};
#ifdef ATLAS_FEATHERED_FILL
void main(){r0+=z6(O,gl_FrontFacing e1);}
#endif
#ifdef ATLAS_FEATHERED_STROKE
void main(){r0=max(r0,C4(O));}
#endif
#elif defined(ATLAS_RENDER_TARGET_R32UI_PLS_ANGLE)
layout(binding=0,r32ui) uniform highp upixelLocalANGLE r0;
#ifdef ATLAS_FEATHERED_FILL
void main(){float o=uintBitsToFloat(pixelLocalLoadANGLE(r0).x);o+=z6(O,gl_FrontFacing e1);pixelLocalStoreANGLE(r0,R(floatBitsToUint(o)));}
#endif
#ifdef ATLAS_FEATHERED_STROKE
void main(){float o=uintBitsToFloat(pixelLocalLoadANGLE(r0).x);o=max(o,C4(O));pixelLocalStoreANGLE(r0,R(floatBitsToUint(o)));}
#endif
#elif defined(ATLAS_RENDER_TARGET_R32I_ATOMIC_TEXTURE)
layout(binding=0,r32i) uniform highp coherent iimage2D Y8;ivec2 ce(){return ivec2(floor(d0));}int de(float o){return int(o*dd);}
#ifdef ATLAS_FEATHERED_FILL
void main(){int o=de(z6(O,gl_FrontFacing e1));imageAtomicAdd(Y8,ce(),o);}
#endif
#ifdef ATLAS_FEATHERED_STROKE
void main(){int o=de(C4(O));imageAtomicMax(Y8,ce(),o);}
#endif
#elif defined(ATLAS_RENDER_TARGET_RGBA8_UNORM)
#ifdef ATLAS_FEATHERED_FILL
v6(i,AF){q(O,f);d o=z6(O,w6 e1);if(abs(o)>cg-1e-3){M2(o>.0?E0(.0,.0,1./255.,.0):E0(.0,.0,.0,1./255.));}else{o*=1./xa;M2(E0(max(o,.0),max(-o,.0),.0,.0));}}
#endif
#ifdef ATLAS_FEATHERED_STROKE
f3(i,BF){q(O,f);d o=C4(O e1);o*=1./xa;M2(E0(o,.0,.0,.0));}
#endif
#else
#ifdef ATLAS_FEATHERED_FILL
v6(float,AF){q(O,f);M2(z6(O,w6 e1));}
#endif
#ifdef ATLAS_FEATHERED_STROKE
f3(float,BF){q(O,f);M2(C4(O e1));}
#endif
#endif
#endif
