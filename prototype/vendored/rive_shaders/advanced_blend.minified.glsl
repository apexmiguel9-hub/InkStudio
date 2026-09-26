#ifdef FRAGMENT
#ifdef ENABLE_KHR_BLEND
layout(
#ifdef ENABLE_HSL_BLEND_MODES
blend_support_all_equations
#else
blend_support_multiply,blend_support_screen,blend_support_overlay,blend_support_darken,blend_support_lighten,blend_support_colordodge,blend_support_colorburn,blend_support_hardlight,blend_support_softlight,blend_support_difference,blend_support_exclusion
#endif
) out;
#endif
#ifdef ENABLE_ADVANCED_BLEND
#ifdef ENABLE_HSL_BLEND_MODES
d Cb(v J1){return dot(J1,T0(.30,.59,.11));}v l9(v Db,v m9){d n9=Cb(m9);v o9=Db-Cb(Db);D Eb=D2(n9,1.0-n9)/max(D2(p9),D2(-m3(o9),N5(o9)));d Ie=min(J0(1.0),min(Eb.x,Eb.y));return o9*Ie+n9;}v Fb(v Q7,v Gb,v m9){float Je=N5(Gb)-m3(Gb);Q7-=m3(Q7);float Ke=N5(Q7);float E2=Je/max(p9,Ke);return l9(Q7*E2,m9);}
#endif
v Le(v p0,i A1,N q9){v w0=F6(A1);v Z0;switch(q9){case Me:Z0=p0.xyz*w0.xyz;break;case Ne:Z0=p0.xyz+w0.xyz-p0.xyz*w0.xyz;break;case Oe:{v G6=p0*w0;Z0=2.0*mix(G6,p0+w0-G6-0.5,greaterThan(w0,T0(0.5)));break;}case Pe:Z0=min(p0.xyz,w0.xyz);break;case Qe:Z0=max(p0.xyz,w0.xyz);break;case Re:{A1.xyz=clamp(A1.xyz,T0(.0),A1.www);v Hb=clamp(1.-p0,T0(.0),T0(1.))*A1.w;Z0=mix(min(T0(1.),A1.xyz/Hb),sign(A1.xyz),equal(Hb,T0(.0)));break;}case Te:{p0=clamp(p0,T0(.0),T0(1.));A1.xyz=clamp(A1.xyz,T0(.0),A1.www);if(A1.w==.0) A1.w=1.;v Ib=A1.w-A1.xyz;Z0=1.-mix(min(T0(1.),Ib/(p0*A1.w)),sign(Ib),equal(p0,T0(.0)));break;}case Ue:{v G6=p0*w0;Z0=2.0*mix(G6,p0+w0-G6-0.5,greaterThan(p0,T0(0.5)));break;}case Ve:{for(int H0=0;H0<3;++H0){if(p0[H0]<=0.5) Z0[H0]=(1.0-w0[H0]);else if(w0[H0]<=0.25) Z0[H0]=((16.0*w0[H0]-12.0)*w0[H0]+3.0);else Z0[H0]=(inversesqrt(w0[H0])-1.0);}Z0=w0+w0*(2.0*p0-1.0)*Z0;break;}case We:Z0=abs(w0.xyz-p0.xyz);break;case Xe:Z0=p0.xyz+w0.xyz-2.*p0.xyz*w0.xyz;break;
#ifdef ENABLE_HSL_BLEND_MODES
case Ye:if(ENABLE_HSL_BLEND_MODES){p0.xyz=clamp(p0.xyz,T0(.0),T0(1.));Z0=Fb(p0.xyz,w0.xyz,w0.xyz);}break;case Ze:if(ENABLE_HSL_BLEND_MODES){p0.xyz=clamp(p0.xyz,T0(.0),T0(1.));Z0=Fb(w0.xyz,p0.xyz,w0.xyz);}break;case af:if(ENABLE_HSL_BLEND_MODES){p0.xyz=clamp(p0.xyz,T0(.0),T0(1.));Z0=l9(p0.xyz,w0.xyz);}break;case bf:if(ENABLE_HSL_BLEND_MODES){p0.xyz=clamp(p0.xyz,T0(.0),T0(1.));Z0=l9(w0.xyz,p0.xyz);}break;
#endif
}return Z0;}e v Y4(v p0,i A1,N q9){v Z0=Le(p0,A1,q9);return mix(p0,Z0,T0(A1.w));}
#endif
#endif
