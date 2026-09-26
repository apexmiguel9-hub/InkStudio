q2
#ifdef USE_FILTERING
I0 W(0,c,c2);
#endif
i2
#ifdef VERTEX
Y3 Z3 F4 G4 h1(g0) i1 B1(KF,g0,F,A,r){c r2;r2.x=(A&1)==0?-1.:1.;r2.y=(A&2)==0?-1.:1.;
#ifdef USE_FILTERING
V(c2,c);c2.x=r2.x*.5+.5;c2.y=r2.y*-.5+.5;c0(c2);
#endif
f X=f(r2,0,1);C1(X);}
#endif
#ifdef FRAGMENT
I3
#ifdef SOURCE_TEXTURE_MSAA
Df(h5,a4,HC);
#else
e3(h5,a4,HC);
#endif
J3
#ifdef USE_FILTERING
i5 c4(Ef) j5
#endif
f3(i,QE){i l8;
#ifdef USE_FILTERING
q(c2,c);l8=U6(HC,Ef,c2,.0);
#elif defined(SOURCE_TEXTURE_MSAA)
l8=(m8(HC,0,Y(floor(d0.xy)))+m8(HC,1,Y(floor(d0.xy)))+m8(HC,2,Y(floor(d0.xy)))+m8(HC,3,Y(floor(d0.xy))))*0.25;
#else
l8=v1(HC,Y(floor(d0.xy)));
#endif
M2(l8);}
#endif
