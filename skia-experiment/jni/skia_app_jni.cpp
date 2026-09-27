// Skia experiment — Ganesh GL canvas + the THREE prototype tools, ported from
// the ThorVG app (prototype/) so the engine comparison is apples-to-apples:
//   Selector (mover): tap = select (8 scale handles, drag to resize/move),
//                     next tap on the selected rect = 4 rotate nodes + center
//                     crosshair (drag to rotate around center),
//                     drag on empty = rubberband
//   Rectangulo      : drag creates a NEW persistent rect (previous ones stay)
//   Nodos           : overlay (8 scale nodes + 4 rotate rings + crosshair),
//                     drag a corner node to resize the rect
//
// Engine: Skia m156 (commit 30ff12f0e3031c536b9b1e4dd073ffe7979f9ecc).
// Every Skia API used is verified against the real headers:
//   SkMatrix::MakeAll / SkCanvas::concat / drawRect|drawLine|drawCircle
//   SkSurfaces::WrapBackendRenderTarget / GrBackendRenderTargets::MakeGL
//   GrDirectContexts::MakeGL / GrGLMakeNativeInterface / skgpu::ganesh::FlushAndSubmit
#include <jni.h>
#include <android/log.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"

#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLTypes.h"

#define LOG_TAG "skiaexp"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace {

// ---- tools (mirror prototype TOOL_SELECT/RECT/NODE id order) --------------
enum Tool { TOOL_SELECT = 0, TOOL_RECT = 1, TOOL_NODE = 2 };

// ---- document: live rect registry (the prototype's sprect_registry) -------
// 2D affine xform, same layout as the prototype's Geom::Xform:
//   x' = a*x + c*y + e ;  y' = b*x + d*y + f
struct Xf {
    float a = 1, b = 0, c = 0, d = 1, e = 0, f = 0;
};
struct R {
    float x = 0, y = 0, w = 0, h = 0;
    Xf xf;
};
std::vector<R> g_rects;

// ---- engine/GL state ------------------------------------------------------
sk_sp<GrDirectContext> g_ctx;
sk_sp<SkSurface> g_surface;
int g_w = 0, g_h = 0;

// ---- interaction state ----------------------------------------------------
Tool g_tool = TOOL_SELECT;
bool g_touching = false;

enum Gest {
    NONE,
    RUB,         // select tool: rubberband on empty area
    MOVE_ITEM,   // select tool: drag inside a selected rect
    SCALE_HAND,  // select tool: drag a scale handle
    ROTATE_ITEM, // select tool: drag a rotate handle (rotate around center)
    CREATE_RECT, // rect tool: drag draws a new rect
    NODE_DRAG,   // node tool: drag a corner node
};
Gest g_gesture = NONE;

int g_sel = -1;               // selected rect (select tool)
int g_target = -1;            // overlay target (node tool)
bool g_rotateState = false;   // select tool handle mode: scale(false)/rotate(true)
int g_handle = -1;            // grabbed handle index
float g_anchorX = 0, g_anchorY = 0;   // move grab: e,f = finger + anchor
float g_cx = 0, g_cy = 0;             // rotate center (doc)
float g_startAng = 0;                 // rotate: start angle at grab
Xf g_grabXf;                          // xform at grab (for rotate delta)

// click-vs-drag bookkeeping
bool g_moved = false;         // finger exceeded the drag tolerance
bool g_newSelDown = true;     // DOWN hit a rect that was NOT already selected
float g_downX = 0, g_downY = 0;
float g_grabOffX = 0, g_grabOffY = 0;  // finger − grabbed point at DOWN (doc)

// rect tool live drag
float g_x0 = 0, g_y0 = 0, g_x1 = 0, g_y1 = 0;
// rubberband (screen)
float g_rub_x0 = 0, g_rub_y0 = 0, g_rub_x1 = 0, g_rub_y1 = 0;

// ---- helpers --------------------------------------------------------------
static void applyXf(const Xf &m, float &x, float &y) {
    float nx = m.a * x + m.c * y + m.e;
    float ny = m.b * x + m.d * y + m.f;
    x = nx; y = ny;
}

// inverse of the 2x2 part, maps doc -> local
static void invXf(const Xf &m, float &x, float &y) {
    float det = m.a * m.d - m.b * m.c;
    if (det == 0.f) return;
    float dx = x - m.e, dy = y - m.f;
    float lx = (m.d * dx - m.c * dy) / det;
    float ly = (-m.b * dx + m.a * dy) / det;
    x = lx; y = ly;
}

// doc (screen) corners of a rect: (x,y),(x+w,y),(x+w,y+h),(x,y+h) xformed
static std::array<SkPoint, 4> docCorners(const R &r) {
    float x0 = r.x, y0 = r.y, x1 = r.x + r.w, y1 = r.y + r.h;
    std::array<SkPoint, 4> c = {{{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}}};
    for (auto &p : c) {
        float xi = p.x(), yi = p.y();
        applyXf(r.xf, xi, yi);
        p = {xi, yi};
    }
    return c;
}

static bool pointInRect(const R &r, float px, float py) {
    invXf(r.xf, px, py);
    return px >= r.x && px <= r.x + r.w && py >= r.y && py <= r.y + r.h;
}

// topmost rect under the finger (registry order = draw order = oldest first)
static int hitRect(float px, float py) {
    for (int i = (int)g_rects.size() - 1; i >= 0; --i) {
        if (g_rects[i].w > 0 && g_rects[i].h > 0 && pointInRect(g_rects[i], px, py)) {
            return i;
        }
    }
    return -1;
}

// 8 scale handle positions (corners + mid-edges) in doc space
static std::array<SkPoint, 8> scaleHandlePositions(const R &r) {
    auto c = docCorners(r);
    std::array<SkPoint, 8> h = {{
        c[0],
        {(c[0].x() + c[1].x()) * 0.5f, (c[0].y() + c[1].y()) * 0.5f},
        c[1],
        {(c[1].x() + c[2].x()) * 0.5f, (c[1].y() + c[2].y()) * 0.5f},
        c[2],
        {(c[2].x() + c[3].x()) * 0.5f, (c[2].y() + c[3].y()) * 0.5f},
        c[3],
        {(c[3].x() + c[0].x()) * 0.5f, (c[3].y() + c[0].y()) * 0.5f},
    }};
    return h;
}

// 4 rotate handles (circles) at 28px diagonal outside corners + center
static void rotateHandlePositions(const R &r, std::array<SkPoint, 4> &h, SkPoint &center) {
    auto c = docCorners(r);
    center = {(c[0].x() + c[1].x() + c[2].x() + c[3].x()) * 0.25f,
              (c[0].y() + c[1].y() + c[2].y() + c[3].y()) * 0.25f};
    const float OFF = 28.0f;
    for (int i = 0; i < 4; ++i) {
        SkPoint next = c[(i + 1) % 4];
        float dx = next.x() - c[i].x();
        float dy = next.y() - c[i].y();
        float len = std::sqrt(dx * dx + dy * dy);
        if (len > 0) { dx = dx / len * OFF; dy = dy / len * OFF; }
        h[i] = {c[i].x() - dx, c[i].y() - dy};
    }
}

static int hitHandle(const std::array<SkPoint, 8> &h, float px, float py, float tol) {
    for (int i = 0; i < 8; ++i) {
        float dx = h[i].x() - px, dy = h[i].y() - py;
        if (dx * dx + dy * dy <= tol * tol) return i;
    }
    return -1;
}

// ---- engine init (identical to the minimal experiment, run 15) ------------
}  // namespace

extern "C" JNIEXPORT jboolean JNICALL
Java_org_inkscape_skia_MainActivity_nativeInit(JNIEnv* env, jobject thiz,
                                               jint w, jint h) {
    SkGraphics::Init();

    sk_sp<const GrGLInterface> iface = GrGLMakeNativeInterface();
    if (!iface) {
        LOGE("nativeInit: GrGLMakeNativeInterface() returned null");
        return JNI_FALSE;
    }
    g_ctx = GrDirectContexts::MakeGL(iface);
    if (!g_ctx) {
        LOGE("nativeInit: GrDirectContexts::MakeGL returned null");
        return JNI_FALSE;
    }
    g_w = w;
    g_h = h;

    GrGLFramebufferInfo fbInfo;
    fbInfo.fFBOID = 0;
    fbInfo.fFormat = 0x8058;  // GL_RGBA8
    GrBackendRenderTarget rt =
            GrBackendRenderTargets::MakeGL(w, h, 0 /*sampleCnt*/, 8 /*stencil*/, fbInfo);

    SkSurfaceProps props;
    g_surface = SkSurfaces::WrapBackendRenderTarget(
            g_ctx.get(), rt, kBottomLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
            nullptr /*colorSpace*/, &props);
    if (!g_surface) {
        LOGE("nativeInit: WrapBackendRenderTarget returned null");
        return JNI_FALSE;
    }
    LOGI("nativeInit OK %dx%d tool=%d rects=%zu", w, h, (int)g_tool, g_rects.size());
    return JNI_TRUE;
}

// ---- tool switching (nativeSetTool, called from the Java toolbar) ---------
extern "C" JNIEXPORT void JNICALL
Java_org_inkscape_skia_MainActivity_nativeSetTool(JNIEnv* env, jobject thiz, jint tool) {
    g_tool = (Tool)tool;
    g_gesture = NONE;
    g_touching = false;
    g_handle = -1;
    LOGI("tool=%d sel=%d target=%d rects=%zu", (int)g_tool, g_sel, g_target,
         g_rects.size());
}

// ---- touch: action 0=DOWN 1=UP 2=MOVE (mirrors the ThorVG app bridge) -----
extern "C" JNIEXPORT void JNICALL
Java_org_inkscape_skia_MainActivity_nativeTouch(JNIEnv* env, jobject thiz,
                                                jint action, jfloat px, jfloat py) {
    if (action == 0) {  // DOWN
        g_downX = px;
        g_downY = py;
        g_moved = false;
        g_newSelDown = false;
        g_touching = true;
        g_gesture = NONE;
        g_handle = -1;

        if (g_tool == TOOL_SELECT) {
            // 1) grabbing a handle of the current selection wins
            if (g_sel >= 0 && g_rects[g_sel].w > 0) {
                R &r = g_rects[g_sel];
                if (!g_rotateState) {
                    auto h = scaleHandlePositions(r);
                    int idx = hitHandle(h, px, py, 26.0f);
                    if (idx >= 0) {
                        g_gesture = SCALE_HAND;
                        g_handle = idx;
                        // grip offset: keep the grabbed point anchored under the
                        // finger so a plain tap resizes NOTHING
                        g_grabOffX = px - h[idx].x();
                        g_grabOffY = py - h[idx].y();
                        return;
                    }
                } else {
                    std::array<SkPoint, 4> h;
                    SkPoint center;
                    rotateHandlePositions(r, h, center);
                    for (int i = 0; i < 4; ++i) {
                        float dx = h[i].x() - px, dy = h[i].y() - py;
                        if (dx * dx + dy * dy <= 26.0f * 26.0f) {
                            g_gesture = ROTATE_ITEM;
                            g_grabXf = r.xf;
                            g_cx = center.x();
                            g_cy = center.y();
                            g_startAng = std::atan2(py - g_cy, px - g_cx);
                            return;
                        }
                    }
                }
            }
            // 2) tap/drag an item (topmost), or rubberband on empty area
            int idx = hitRect(px, py);
            if (idx >= 0) {
                g_newSelDown = (idx != g_sel);
                g_sel = idx;
                g_gesture = MOVE_ITEM;
                g_anchorX = g_rects[idx].xf.e - px;
                g_anchorY = g_rects[idx].xf.f - py;
            } else {
                g_sel = -1;
                g_rotateState = false;
                g_gesture = RUB;
                g_rub_x0 = g_rub_x1 = px;
                g_rub_y0 = g_rub_y1 = py;
            }
        } else if (g_tool == TOOL_RECT) {
            g_gesture = CREATE_RECT;
            g_x0 = g_x1 = px;
            g_y0 = g_y1 = py;
        } else if (g_tool == TOOL_NODE) {
            int idx = hitRect(px, py);
            if (idx >= 0) {
                g_target = idx;
                auto h = scaleHandlePositions(g_rects[idx]);
                int hidx = hitHandle(h, px, py, 26.0f);
                if (hidx >= 0) {
                    g_gesture = NODE_DRAG;
                    g_handle = hidx;
                    g_grabOffX = px - h[hidx].x();
                    g_grabOffY = py - h[hidx].y();
                }
            }
        }
    } else if (action == 2 && g_touching) {  // MOVE
        switch (g_gesture) {
            case RUB:
                g_rub_x1 = px;
                g_rub_y1 = py;
                break;
            case CREATE_RECT:
                g_x1 = px;
                g_y1 = py;
                break;
            case MOVE_ITEM: {
                // only translate after the finger passes the drag tolerance,
                // so a plain tap never nudges the rect
                float dx = px - g_downX, dy = py - g_downY;
                if (dx * dx + dy * dy > 8.0f * 8.0f) {
                    g_moved = true;
                    if (g_sel >= 0) {
                        g_rects[g_sel].xf.e = px + g_anchorX;
                        g_rects[g_sel].xf.f = py + g_anchorY;
                    }
                }
                break;
            }
            case SCALE_HAND:
            case NODE_DRAG: {
                int idx = (g_gesture == NODE_DRAG) ? g_target : g_sel;
                if (idx < 0 || g_handle < 0) break;
                // resize only once the finger really slides (tap = no change)
                float ddx = px - g_downX, ddy = py - g_downY;
                if (ddx * ddx + ddy * ddy <= 8.0f * 8.0f) break;
                g_moved = true;
                R &rr = g_rects[idx];
                // dragged point (doc) = finger minus the grip offset, so the
                // grabbed handle stays exactly where the finger grabbed it
                float docx = px - g_grabOffX, docy = py - g_grabOffY;
                float lx = docx, ly = docy;
                invXf(rr.xf, lx, ly);
                int h = g_handle;
                if (h % 2 == 0) {
                    // corner handle: opposite corner stays fixed (local space)
                    int c = h / 2;
                    int fix = (c + 2) % 4;  // 0=TL 1=TR 2=BR 3=BL
                    float fxl = (fix == 0 || fix == 3) ? rr.x : rr.x + rr.w;
                    float fyl = (fix == 0 || fix == 1) ? rr.y : rr.y + rr.h;
                    float l0 = std::min(fxl, lx), l1 = std::max(fxl, lx);
                    float t0 = std::min(fyl, ly), t1 = std::max(fyl, ly);
                    if (l1 - l0 < 8.0f) l1 = l0 + 8.0f;
                    if (t1 - t0 < 8.0f) t1 = t0 + 8.0f;
                    rr.x = l0;
                    rr.y = t0;
                    rr.w = l1 - l0;
                    rr.h = t1 - t0;
                } else {
                    // mid-edge handle: that edge follows the finger, the
                    // opposite edge stays fixed
                    int e = h / 2;  // 0=top 1=right 2=bottom 3=left
                    if (e == 0 || e == 2) {
                        float fix = (e == 0) ? (rr.y + rr.h) : rr.y;
                        float y0 = std::min(fix, ly), y1 = std::max(fix, ly);
                        if (y1 - y0 < 8.0f) y1 = y0 + 8.0f;
                        rr.y = y0;
                        rr.h = y1 - y0;
                    } else {
                        // e=1 right edge: left stays; e=3 left edge: right stays
                        float fix = (e == 1) ? rr.x : (rr.x + rr.w);
                        float x0 = std::min(fix, lx), x1 = std::max(fix, lx);
                        if (x1 - x0 < 8.0f) x1 = x0 + 8.0f;
                        rr.x = x0;
                        rr.w = x1 - x0;
                    }
                }
                break;
            }
            case ROTATE_ITEM: {
                if (g_sel < 0) break;
                // plain tap (jitter under tolerance) never rotates
                float ddx = px - g_downX, ddy = py - g_downY;
                if (ddx * ddx + ddy * ddy <= 8.0f * 8.0f) break;
                g_moved = true;
                float ang = std::atan2(py - g_cy, px - g_cx);
                float dtheta = ang - g_startAng;
                // M' = T(C)·R(θ)·T(-C)·M
                float c = std::cos(dtheta), s = std::sin(dtheta);
                const Xf &m = g_grabXf;
                Xf A;  // T(-C)·M
                A.a = m.a; A.b = m.b; A.c = m.c; A.d = m.d;
                A.e = m.e - g_cx; A.f = m.f - g_cy;
                Xf B;  // R(θ)·A
                B.a = c * A.a - s * A.b;
                B.b = s * A.a + c * A.b;
                B.c = c * A.c - s * A.d;
                B.d = s * A.c + c * A.d;
                B.e = c * A.e - s * A.f;
                B.f = s * A.e + c * A.f;
                // T(C)·B
                g_rects[g_sel].xf = B;
                g_rects[g_sel].xf.e += g_cx;
                g_rects[g_sel].xf.f += g_cy;
                break;
            }
            default:
                break;
        }
    } else if (action == 1 && g_touching) {  // UP/CANCEL
        g_touching = false;
        switch (g_gesture) {
            case MOVE_ITEM:
                // plain tap (no drag) on the ALREADY-selected rect toggles the
                // handle mode: 1st tap = 8 scale handles, next tap = 4 rotate
                // nodes (Inkscape-style selection cycling, not a timed
                // double-tap)
                if (!g_moved && !g_newSelDown && g_sel >= 0) {
                    g_rotateState = !g_rotateState;
                }
                break;
            case RUB: {
                // select the topmost rect fully inside the rubber, else clear
                float x0 = std::min(g_rub_x0, g_rub_x1);
                float y0 = std::min(g_rub_y0, g_rub_y1);
                float x1 = std::max(g_rub_x0, g_rub_x1);
                float y1 = std::max(g_rub_y0, g_rub_y1);
                int hit = -1;
                for (int i = (int)g_rects.size() - 1; i >= 0; --i) {
                    R &r = g_rects[i];
                    if (r.w <= 0 || r.h <= 0) continue;
                    auto dc = docCorners(r);
                    float minx = dc[0].x(), maxx = dc[0].x();
                    float miny = dc[0].y(), maxy = dc[0].y();
                    for (auto &p : dc) {
                        minx = std::min(minx, p.x()); maxx = std::max(maxx, p.x());
                        miny = std::min(miny, p.y()); maxy = std::max(maxy, p.y());
                    }
                    if (minx >= x0 && maxx <= x1 && miny >= y0 && maxy <= y1) {
                        hit = i;
                        break;
                    }
                }
                g_sel = hit;
                g_rotateState = false;
                break;
            }
            case CREATE_RECT: {
                float x0 = std::min(g_x0, g_x1), y0 = std::min(g_y0, g_y1);
                float x1 = std::max(g_x0, g_x1), y1 = std::max(g_y0, g_y1);
                if (x1 - x0 >= 8.0f && y1 - y0 >= 8.0f) {
                    R nr;
                    nr.x = x0;
                    nr.y = y0;
                    nr.w = x1 - x0;
                    nr.h = y1 - y0;
                    g_rects.push_back(nr);
                    g_target = (int)g_rects.size() - 1;
                    LOGI("rect created %dx%d+%dx%d n=%zu", (int)x0, (int)y0,
                         (int)nr.w, (int)nr.h, g_rects.size());
                }
                break;
            }
            default:
                break;
        }
        g_gesture = NONE;
        g_handle = -1;
    }
}

// ---- drawing --------------------------------------------------------------
static void drawHandleSquares(SkCanvas *canvas, const std::array<SkPoint, 8> &h) {
    const float HALF = 11.0f;  // 22x22 px grab nodes (were 10px: too small)
    SkPaint sq;
    sq.setAntiAlias(true);
    sq.setStyle(SkPaint::kFill_Style);
    sq.setColor(SkColorSetARGB(255, 255, 255, 255));
    SkPaint edge;
    edge.setAntiAlias(true);
    edge.setStyle(SkPaint::kStroke_Style);
    edge.setStrokeWidth(1.0f);
    edge.setColor(SkColorSetARGB(255, 0x10, 0x10, 0x14));
    for (auto &p : h) {
        SkRect r = SkRect::MakeXYWH(p.x() - HALF, p.y() - HALF, HALF * 2, HALF * 2);
        canvas->drawRect(r, sq);
        canvas->drawRect(r, edge);
    }
}

static void drawRotateRings(SkCanvas *canvas, const std::array<SkPoint, 4> &rh,
                            const SkPoint &center) {
    const float RAD = 12.0f;  // bigger grab (was 7px: too small)
    SkPaint ring;
    ring.setAntiAlias(true);
    ring.setStyle(SkPaint::kFill_Style);
    ring.setColor(SkColorSetARGB(255, 255, 255, 255));
    SkPaint edge;
    edge.setAntiAlias(true);
    edge.setStyle(SkPaint::kStroke_Style);
    edge.setStrokeWidth(1.0f);
    edge.setColor(SkColorSetARGB(255, 0x10, 0x10, 0x14));
    for (auto &p : rh) {
        canvas->drawCircle(p.x(), p.y(), RAD, ring);
        canvas->drawCircle(p.x(), p.y(), RAD, edge);
    }
    SkPaint cross;
    cross.setAntiAlias(true);
    cross.setStyle(SkPaint::kFill_Style);
    cross.setColor(SkColorSetARGB(255, 0x10, 0x10, 0x14));
    canvas->drawRect(SkRect::MakeXYWH(center.x() - 8.0f, center.y() - 0.75f, 16.0f, 1.5f), cross);
    canvas->drawRect(SkRect::MakeXYWH(center.x() - 0.75f, center.y() - 8.0f, 1.5f, 16.0f), cross);
}

// node tool overlay: 8 scale squares + 4 rotate rings + crosshair (the
// prototype's drawNodeOverlay)
static void drawNodeOverlay(SkCanvas *canvas, const R &r) {
    auto h = scaleHandlePositions(r);
    drawHandleSquares(canvas, h);
    std::array<SkPoint, 4> rh;
    SkPoint center;
    rotateHandlePositions(r, rh, center);
    drawRotateRings(canvas, rh, center);
}

extern "C" JNIEXPORT void JNICALL
Java_org_inkscape_skia_MainActivity_nativeDraw(JNIEnv* env, jobject thiz) {
    if (!g_surface) return;
    SkCanvas *canvas = g_surface->getCanvas();

    canvas->clear(SkColorSetARGB(255, 0xEF, 0xEF, 0xF4));

    SkPaint fill;
    fill.setAntiAlias(true);
    fill.setStyle(SkPaint::kFill_Style);
    fill.setColor(SkColorSetARGB(240, 0x33, 0x66, 0xFF));
    SkPaint stroke;
    stroke.setAntiAlias(true);
    stroke.setStyle(SkPaint::kStroke_Style);
    stroke.setStrokeWidth(1.5f);
    stroke.setColor(SkColorSetARGB(255, 0x12, 0x14, 0x20));

    // registry rects, each with its document transform
    for (const R &r : g_rects) {
        if (r.w <= 0 || r.h <= 0) continue;
        SkRect lr = SkRect::MakeXYWH(r.x, r.y, r.w, r.h);
        canvas->save();
        canvas->concat(SkMatrix::MakeAll(r.xf.a, r.xf.c, r.xf.e,
                                         r.xf.b, r.xf.d, r.xf.f, 0.f, 0.f, 1.f));
        canvas->drawRect(lr, fill);
        canvas->drawRect(lr, stroke);
        canvas->restore();
    }

    // rect tool live drag preview (not committed until UP)
    if (g_tool == TOOL_RECT && g_gesture == CREATE_RECT) {
        float x0 = std::min(g_x0, g_x1), y0 = std::min(g_y0, g_y1);
        float x1 = std::max(g_x0, g_x1), y1 = std::max(g_y0, g_y1);
        if (x1 - x0 >= 1.f && y1 - y0 >= 1.f) {
            canvas->drawRect(SkRect::MakeLTRB(x0, y0, x1, y1), fill);
            canvas->drawRect(SkRect::MakeLTRB(x0, y0, x1, y1), stroke);
        }
    }

    // selection overlay (select tool)
    if (g_tool == TOOL_SELECT && g_sel >= 0 && g_rects[g_sel].w > 0) {
        R &r = g_rects[g_sel];
        auto c = docCorners(r);
        SkPaint cue;
        cue.setAntiAlias(true);
        cue.setStyle(SkPaint::kStroke_Style);
        cue.setStrokeWidth(1.0f);
        cue.setColor(SkColorSetARGB(255, 0x2A, 0x2D, 0x35));
        for (int i = 0; i < 4; ++i) {
            SkPoint a = c[i], b = c[(i + 1) % 4];
            canvas->drawLine(a.x(), a.y(), b.x(), b.y(), cue);
        }
        if (!g_rotateState) {
            drawHandleSquares(canvas, scaleHandlePositions(r));
        } else {
            std::array<SkPoint, 4> rh;
            SkPoint center;
            rotateHandlePositions(r, rh, center);
            drawRotateRings(canvas, rh, center);
        }
    }

    // node tool overlay
    if (g_tool == TOOL_NODE && g_target >= 0 && g_target < (int)g_rects.size()) {
        drawNodeOverlay(canvas, g_rects[g_target]);
    }

    // rubberband
    if (g_gesture == RUB) {
        float x0 = std::min(g_rub_x0, g_rub_x1), y0 = std::min(g_rub_y0, g_rub_y1);
        float x1 = std::max(g_rub_x0, g_rub_x1), y1 = std::max(g_rub_y0, g_rub_y1);
        SkPaint rf;
        rf.setAntiAlias(true);
        rf.setStyle(SkPaint::kFill_Style);
        rf.setColor(SkColorSetARGB(60, 0x33, 0x66, 0xFF));
        SkPaint rs;
        rs.setAntiAlias(true);
        rs.setStyle(SkPaint::kStroke_Style);
        rs.setStrokeWidth(1.0f);
        rs.setColor(SkColorSetARGB(255, 0x2E, 0x5F, 0xFF));
        canvas->drawRect(SkRect::MakeLTRB(x0, y0, x1, y1), rf);
        canvas->drawRect(SkRect::MakeLTRB(x0, y0, x1, y1), rs);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_org_inkscape_skia_MainActivity_nativePresent(JNIEnv* env, jobject thiz) {
    if (g_surface) {
        skgpu::ganesh::FlushAndSubmit(g_surface.get());
    }
}