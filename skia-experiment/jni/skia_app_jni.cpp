// Skia experiment — Ganesh GL canvas + rect tool with tap+drag.
//
// Port of the minimal Rive/ThorVG experiment to Skia (m156, commit
// 30ff12f0e3031c536b9b1e4dd073ffe7979f9ecc, google/skia main 2026-09-27).
// Every API used here was verified against the real headers:
//   include/core/SkGraphics.h            -> SkGraphics::Init()
//   include/gpu/ganesh/SkSurfaceGanesh.h -> SkSurfaces::WrapBackendRenderTarget(...)
//                                           skgpu::ganesh::FlushAndSubmit(...)
//   include/gpu/ganesh/gl/GrGLDirectContext.h    -> GrDirectContexts::MakeGL(...)
//   include/gpu/ganesh/gl/GrGLBackendSurface.h   -> GrBackendRenderTargets::MakeGL(...)
//   include/gpu/ganesh/gl/GrGLInterface.h        -> GrGLMakeNativeInterface()
//   include/gpu/ganesh/gl/GrGLTypes.h            -> GrGLFramebufferInfo {fFBOID; fFormat}
//   include/gpu/ganesh/GrTypes.h                 -> kBottomLeft_GrSurfaceOrigin
#include <jni.h>
#include <android/log.h>
#include <algorithm>
#include <cmath>

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
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

sk_sp<GrDirectContext> g_ctx;
sk_sp<SkSurface> g_surface;

int g_w = 0;
int g_h = 0;

// Current rect being created by tap+drag (device pixels, canvas coords).
float g_x0 = -1;
float g_y0 = -1;
float g_x1 = -1;
float g_y1 = -1;
bool g_touching = false;

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

    // Wrap the GLSurfaceView default framebuffer (FBO 0) as an SkSurface.
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

    LOGI("nativeInit OK %dx%d", w, h);
    return JNI_TRUE;
}

extern "C" JNIEXPORT void JNICALL
Java_org_inkscape_skia_MainActivity_nativeDraw(JNIEnv* env, jobject thiz) {
    if (!g_surface) {
        return;
    }
    SkCanvas* canvas = g_surface->getCanvas();

    // Same palette as the ThorVG prototype: light canvas, navy rect.
    canvas->clear(SkColorSetARGB(255, 0xEF, 0xEF, 0xF4));

    if (g_x0 >= 0 && g_y0 >= 0 && g_x1 >= 0 && g_y1 >= 0) {
        float l = std::min(g_x0, g_x1);
        float t = std::min(g_y0, g_y1);
        float r = std::max(g_x0, g_x1);
        float b = std::max(g_y0, g_y1);
        if (r - l < 1.f || b - t < 1.f) {
            return;
        }

        SkRect rect = SkRect::MakeLTRB(l, t, r, b);
        // m156: SkPath is data-oriented; classic addRect/moveTo/lineTo are gone.
        // Verified: include/core/SkPath.h has static factories only (Rect,
        // Polygon, Line, Raw). https://github.com/google/skia/blob/main/include/core/SkPath.h
        SkPath path = SkPath::Rect(rect);

        SkPaint fill;
        fill.setAntiAlias(true);
        fill.setStyle(SkPaint::kFill_Style);
        fill.setColor(SkColorSetARGB(240, 0x33, 0x66, 0xFF));
        canvas->drawPath(path, fill);

        SkPaint stroke;
        stroke.setAntiAlias(true);
        stroke.setStyle(SkPaint::kStroke_Style);
        stroke.setStrokeWidth(1.5f);
        stroke.setColor(SkColorSetARGB(255, 0x12, 0x14, 0x20));
        canvas->drawPath(path, stroke);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_org_inkscape_skia_MainActivity_nativePresent(JNIEnv* env, jobject thiz) {
    if (g_surface) {
        skgpu::ganesh::FlushAndSubmit(g_surface.get());
    }
}

// action: 0=DOWN, 1=UP, 2=MOVE, 3=CANCEL (mapped in MainActivity.java).
extern "C" JNIEXPORT void JNICALL
Java_org_inkscape_skia_MainActivity_nativeTouch(JNIEnv* env, jobject thiz,
                                                jint action, jfloat x, jfloat y) {
    if (action == 0) {
        g_touching = true;
        g_x0 = x;
        g_y0 = y;
        g_x1 = x;
        g_y1 = y;
    } else if (action == 2) {
        if (g_touching) {
            g_x1 = x;
            g_y1 = y;
        }
    } else if (action == 1 || action == 3) {
        g_touching = false;
    }
}