#pragma once

#include "specialization.glsl.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char specialization[] = R"===(layout(constant_id=qg) const bool Hh=true;layout(constant_id=rg) const bool Ih=true;layout(constant_id=sg) const bool Jh=true;layout(constant_id=tg) const bool Kh=true;layout(constant_id=ug) const bool Lh=true;layout(constant_id=vg) const bool Mh=true;layout(constant_id=wg) const bool Nh=true;layout(constant_id=xg) const bool Oh=true;layout(constant_id=yg) const bool Ph=true;layout(constant_id=zg) const bool Qh=true;layout(constant_id=Ag) const bool Rh=false;layout(constant_id=Bg) const bool Sh=false;layout(constant_id=Cg) const bool Th=false;layout(constant_id=Dg) const bool Uh=false;layout(constant_id=Eg) const bool Vh=false;layout(constant_id=Fg) const bool Wh=false;
#define I Hh
#define AB Ih
#define T Jh
#define GB Kh
#define WC Lh
#define YC Mh
#define DC Nh
#define LB Oh
#define IB Ph
#define GE Qh
#define JD Rh
#define CC Sh
#define QD Uh
#define RD Vh
#define GD Wh
#ifndef BE
#if defined(BB)
#define KD Th
#endif
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive