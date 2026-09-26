layout(constant_id=qg) const bool Hh=true;layout(constant_id=rg) const bool Ih=true;layout(constant_id=sg) const bool Jh=true;layout(constant_id=tg) const bool Kh=true;layout(constant_id=ug) const bool Lh=true;layout(constant_id=vg) const bool Mh=true;layout(constant_id=wg) const bool Nh=true;layout(constant_id=xg) const bool Oh=true;layout(constant_id=yg) const bool Ph=true;layout(constant_id=zg) const bool Qh=true;layout(constant_id=Ag) const bool Rh=false;layout(constant_id=Bg) const bool Sh=false;layout(constant_id=Cg) const bool Th=false;layout(constant_id=Dg) const bool Uh=false;layout(constant_id=Eg) const bool Vh=false;layout(constant_id=Fg) const bool Wh=false;
#define ENABLE_CLIPPING Hh
#define ENABLE_CLIP_RECT Ih
#define ENABLE_ADVANCED_BLEND Jh
#define ENABLE_FEATHER Kh
#define ENABLE_EVEN_ODD Lh
#define ENABLE_NESTED_CLIPPING Mh
#define ENABLE_HSL_BLEND_MODES Nh
#define ENABLE_DITHER Oh
#define ENABLE_MODULATED_IMAGE Ph
#define CLOCKWISE_FILL Qh
#define NESTED_CLIP_UPDATE_ONLY Rh
#define BORROWED_COVERAGE_PASS Sh
#define STORE_COLOR_CLEAR Uh
#define LOAD_COLOR_FROM_DST_TEXTURE Vh
#define VULKAN_VENDOR_ARM Wh
#ifndef TARGET_WGSL
#if defined(RENDER_MODE_DEPTH_STENCIL)
#define EMULATE_DYNAMIC_COLOR_WRITE_DISABLE Th
#endif
#endif
