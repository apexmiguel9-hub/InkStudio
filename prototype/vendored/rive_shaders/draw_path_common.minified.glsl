#define j7 -2.
#define ed -1.5
#define fd .25
#define C8 1e3
#define gd (C8*C8)
#ifdef VERTEX
Y3 L4(h3,jg,JC);
#ifdef ENABLE_FEATHER
h6(h3,h7,XC);
#endif
Z3 F4 M4(Yc,Hg,OB);O5(Qb,kf,CD);P5(Rb,lf,PB);M4(Zc,Ig,HD);G4
#endif
#if defined(ENABLE_FEATHER)||defined(FEATHER_ATLAS_BLIT)
g4(h7,ca)
#endif
#ifdef FRAGMENT
I3 e3(h3,ad,DD);
#if defined(ENABLE_FEATHER)||defined(FEATHER_ATLAS_BLIT)
h6(h3,h7,XC);
#endif
#ifdef FEATHER_ATLAS_BLIT
o5(h3,bd,ED);
#endif
e3(h5,a4,GC);
#if defined(RENDER_MODE_DEPTH_STENCIL)&&defined(ENABLE_ADVANCED_BLEND)&&!defined(FIXED_FUNCTION_COLOR_OUTPUT)
p5(XD);
#endif
J3 g4(ad,M9)
#ifdef FEATHER_ATLAS_BLIT
g4(bd,R9)
#endif
i5 c4(W5) j5
#endif
#ifdef FRAGMENT
e bool V5(f P){return P.y>=.0;}e bool V5(D P){return P.y>=.0;}
#endif
#if defined(FRAGMENT)&&defined(ENABLE_FEATHER)
e bool bc(f P){return P.x<ed;}e bool cc(f P){return P.y<ed;}
#endif
#ifdef VERTEX
f hd(float za,c D8,float G1){c i6=(1.-D8*abs(G1))*.5;float h4,q5;if(abs(za-X6)<1./C8){h4=.0;q5=.0;}else{float Aa=tan(za);h4=sign(X6-za)/max(abs(Aa),1./gd);q5=h4>=.0?i6.y-(1.-i6.x)*Aa:i6.y+i6.x*Aa;}f P;P.x=max(i6.x,.0)+fd;P.y=-i6.y+j7;P.z=h4;P.w=q5;return P;}
#endif
#ifdef ENABLE_FEATHER
e d d8(f P L3){d h4=P.z;d q5=max(P.w,.0);d j6=h4>=.0?l5(q5):.0;if(abs(h4)<C8){d x=abs(P.x)-fd;d y=-P.y+j7;d c3=(y-q5)*0.5984134206;i t=q5+c3*E0(0.20888568955,0.62665706865,1.04442844776,1.46219982687);i u=t*-h4+(y*h4+x);i Jg=E0(l5(u[0]),l5(u[1]),l5(u[2]),l5(u[3]));i id=t*5.09593080173+-2.54796540086;i Kg=exp2(-id*id);j6+=dot(Jg,Kg)*c3;}return j6*sign(P.x);}e d C4(f P L3){float j6=1.;float Lg=(1.-j7)+P.x;j6-=l5(Lg);float Mg=1.-P.y;j6-=l5(Mg);return j6;}
#endif
#if defined(VERTEX)&&defined(DRAW_PATH)
e Y r5(int jd){return Y(jd&((1<<Nc)-1),jd>>Nc);}e float kd(e0 W0,c Ng){c m2=P0(W0,Ng);return(abs(m2.x)+abs(m2.y))*(1./dot(m2,m2));}e bool r9(f k7,f Ba,int r,c1(uint) i3,c1(c) Og
#ifndef RENDER_MODE_DEPTH_STENCIL
,c1(f) S1
#else
,c1(N) l7
#endif
k6){int E8=int(k7.x);float G1=k7.y;float Ca=k7.z;int ld=floatBitsToInt(k7.w)>>2;int m7=floatBitsToInt(k7.w)&3;int Da=min(E8,ld-1);int N4=r*ld+Da;R v5=v1(JC,r5(N4));uint j0=v5.w;uint F8=max(j0&Uc,1u);R Ea=L0(HD,F8-1u);c md=uintBitsToFloat(Ea.xy);i3=Ea.z&0xffffu;uint nd=Ea.w;e0 W0=L1(uintBitsToFloat(L0(OB,i3*4u)));R O4=L0(OB,i3*4u+1u);c I2=uintBitsToFloat(O4.xy);float N2=uintBitsToFloat(O4.z);float O2=uintBitsToFloat(O4.w);uint od=j0&K3;if(od!=0u){E8=int(Ba.x);G1=Ba.y;Ca=Ba.z;}if(E8!=Da){int pd=N4+E8-Da;R qd=v1(JC,r5(pd));if((qd.w&(K3|0xffffu))!=(j0&(K3|0xffffu))){bool Pg=N2==.0||md.x!=.0;if(Pg){N4=int(nd);v5=v1(JC,r5(N4));}}else{N4=pd;v5=qd;}j0=(v5.w&~K3)|od;}bool Fa=false;float f1;
#ifdef ENABLE_FEATHER
float n7;float x1;if((j0&f4)==y8&&m7==B8){uint rd=v5.z;float i4=float(rd&0xffffu);float n2=float(rd>>16);Y G8=Y(-i4-1.,n2-i4+1.);if((j0&K3)!=0u) G8=-G8;R sd=v1(JC,r5(N4+G8.x));R Ga=v1(JC,r5(N4+G8.y));if((Ga.w&(K3|0xffffu))!=(sd.w&(K3|0xffffu))){Ga=v1(JC,r5(int(nd)));}n7=uintBitsToFloat(sd.z);float td=uintBitsToFloat(Ga.z);x1=td-n7;if(abs(x1)>H3) x1-=p8*sign(x1);float Ha=n2+1.-float(Oc);float ud=clamp(round(abs(x1)/H3*Ha),1.,Ha-1.);float o7=Ha-ud;if(i4<=o7){x1=-(H3*sign(x1)-x1);n2=o7;if(i4==o7) G1=-G1;}else if(i4==o7+1.){i4=.0;n2=.0;G1=.0;}else{i4-=o7+2.;n2=ud;}if(i4==n2){f1=td;}else{f1=n7+x1*(i4/n2);}}else
#endif
{f1=uintBitsToFloat(v5.z);}c d3=c(sin(f1),-cos(f1));c vd=uintBitsToFloat(v5.xy);c H8=c(0,0);if(O2!=.0){O2=max(O2,(qa/3.)/length(P0(W0,d3)));}if(N2!=.0){G1*=sign(determinant(W0));if((j0&A8)!=0u) G1=min(G1,.0);if((j0&Tc)!=0u) G1=max(G1,.0);float P4=O2!=.0?O2:kd(W0,d3)*x4;d wd=1.;if(P4>N2&&O2==.0){wd=S3(N2)/S3(P4);N2=P4;}c w5=d3*(N2+P4);
#ifndef RENDER_MODE_DEPTH_STENCIL
float x=G1*(N2+P4);S1.xy=(1./(P4*2.))*(c(x,-x)+N2)+.5;S1.zw=N6(.0);
#endif
uint Ia=j0&f4;if(Ia>x8){int p7=2;if((j0&ra)==0u) p7=-p7;if((j0&K3)!=0u) p7=-p7;Y Qg=r5(N4+p7);R Rg=v1(JC,Qg);float Sg=uintBitsToFloat(Rg.z);float q7=abs(Sg-f1);if(q7>H3) q7=p8-q7;bool I8=(j0&ra)!=0u;bool Tg=(j0&A8)!=0u;float xd=q7*(I8==Tg?-.5:.5)+f1;c J8=c(sin(xd),-cos(xd));float Ja=kd(W0,J8);float r7=cos(q7*.5);float Ka;if((Ia==eg)||(Ia==fg&&r7>=.25)){float Ug=(j0&z8)!=0u?1.:.25;Ka=N2*(1./max(r7,Ug));}else{Ka=N2*r7+Ja*.5;}float La=Ka+Ja*x4;if((j0&Sc)!=0u){float yd=N2+P4;float Vg=P4*.125;if(yd<=La*r7+Vg){float Wg=yd*(1./r7);w5=J8*Wg;}else{c Ma=J8*La;c Xg=c(dot(w5,w5),dot(Ma,Ma));w5=P0(Xg,inverse(e0(w5,Ma)));}}c Yg=abs(G1)*w5;float zd=(La-dot(Yg,J8))/(Ja*(x4*2.));
#ifndef RENDER_MODE_DEPTH_STENCIL
if((j0&A8)!=0u) S1.y=zd;else S1.x=zd;
#endif
}
#ifndef RENDER_MODE_DEPTH_STENCIL
S1.xy*=wd;S1.y=max(S1.y,1e-4);if(O2!=.0){S1.x=j7-S1.x;}
#endif
H8=P0(W0,G1*w5);if(m7!=B8) Fa=true;}else{
#ifndef RENDER_MODE_DEPTH_STENCIL
S1=f(Ca,-1.,.0,.0);
#ifdef ENABLE_FEATHER
if(O2!=.0){S1.y=j7;S1.z=gd;S1.w=Ca;if((j0&f4)==y8&&m7==B8){if(x1<.0){n7+=x1;x1=-x1;}float j4=f1-n7;j4=mod(j4+X6,p8)-X6;j4=clamp(j4,.0,x1);if(j4>x1*.5){j4=x1-j4;}c D8=c(sin(j4),cos(j4));
#if 0
float T1=1.+.33*log2(X6/(H3-min(x1,H3-H3/16.)));f Zg=hd(x1,D8,.5*(T1/3.));float ah=d8(Zg e1);float bh=zc(ah);float ch=(.5-bh)*(qa*2.);float dh=T1/max(ch,T1);G1*=dh;
#endif
S1=hd(x1,D8,G1);}H8=P0(W0,(G1*O2)*d3);}else
#endif
{H8=sign(P0(G1*d3,inverse(W0)))*x4;}if(bool(j0&K3)!=bool(j0&gg)){S1*=f(-1.,+1.,+1.,+1.);}
#endif
if(m7==Wc) vd=md;if((j0&Rc)!=0u&&m7!=Vc){Fa=true;}}Og=P0(W0,vd)+H8+I2;
#ifdef RENDER_MODE_DEPTH_STENCIL
R Q4=L0(OB,i3*4u+2u);l7=a2(Q4.x);
#else
S1.xy=mix(S1.xy,c(1.,-1.),Lf(j.eh!=0u));
#endif
return!Fa;}
#endif
#if defined(VERTEX)&&defined(DRAW_INTERIOR_TRIANGLES)
e c Kb(S l6,c1(uint) i3
#ifdef RENDER_MODE_DEPTH_STENCIL
,c1(N) l7
#else
,c1(d) fh
#endif
k6){i3=floatBitsToUint(l6.z)&0xffffu;
#ifdef RENDER_MODE_DEPTH_STENCIL
R Q4=L0(OB,i3*4u+2u);l7=a2(Q4.x);
#else
fh=ea(floatBitsToInt(l6.z)>>16);
#endif
c m6=l6.xy;e0 W0=L1(uintBitsToFloat(L0(OB,i3*4u)));R O4=L0(OB,i3*4u+1u);c I2=uintBitsToFloat(O4.xy);m6=P0(W0,m6)+I2;return m6;}
#endif
#if defined(VERTEX)&&defined(FEATHER_ATLAS_BLIT)
e c Jb(S l6,c1(uint) i3,
#ifdef RENDER_MODE_DEPTH_STENCIL
c1(N) l7,
#endif
c1(c) gh k6){i3=floatBitsToUint(l6.z)&0xffffu;R Q4=L0(OB,i3*4u+2u);
#ifdef RENDER_MODE_DEPTH_STENCIL
l7=a2(Q4.x);
#endif
c m6=l6.xy;S v7=uintBitsToFloat(Q4.yzw);gh=(m6*v7.x+v7.yz)*j.hh;return m6;}
#endif
e d K8(d e2,d H1,d j3){return(H1-e2)/max(1.-e2*j3,p9);}
#if defined(RENDER_MODE_CLOCKWISE_ATOMIC)||defined(PLS_IMPL_STORAGE_BUFFER)
e uint L8(N0 k4,uint ih){uint Na=(k4.y>>g6)*(ih<<g6)+((k4.x>>g6)<<(g6<<1));Na+=((k4.x&0x1cu)<<g6)+((k4.y&0x1cu)<<2);Na+=((k4.y&0x3u)<<2)+(k4.x&0x3u);return Na;}
#endif
#ifdef RENDER_MODE_CLOCKWISE_ATOMIC
#ifdef FIXED_FUNCTION_COLOR_OUTPUT
#define k5 v2
#define d4(x5) F1=x5;r3
#else
#define k5 P1
#define d4(x5) A0(m0,x5);d2;
#endif
e d Oa(uint jh){return ea(int((jh&wa)-n5))*ua;}e uint w7(d o){return uint(o*pg+.5);}
#endif
