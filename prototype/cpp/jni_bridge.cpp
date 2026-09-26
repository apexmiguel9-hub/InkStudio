// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * jni_bridge.cpp — JNI surface exposed to MainActivity (Rive experiment).
 *
 * Static-method JNI (no instance state); the Renderer is a process-lifetime
 * singleton. All calls are enqueued to the render thread by the Renderer —
 * the UI thread never touches tool/registry/Vulkan state directly.
 */
#include <jni.h>

#include <android/native_window_jni.h>

#include "renderer.h"

static Renderer *renderer()
{
    static Renderer *r = new Renderer();
    return r;
}

extern "C" JNIEXPORT void JNICALL
Java_org_inkscape_alpha_MainActivity_nativeInit(JNIEnv *, jclass)
{
    renderer()->init();
}

// The SurfaceView's Surface (or null when the surface is destroyed).
// ANativeWindow_fromSurface() returns a new reference every call; the
// Renderer owns and releases it.
extern "C" JNIEXPORT void JNICALL
Java_org_inkscape_alpha_MainActivity_nativeSurface(JNIEnv *env, jclass,
                                                   jobject jsurface)
{
    ANativeWindow *window = nullptr;
    if (jsurface != nullptr) {
        window = ANativeWindow_fromSurface(env, jsurface);
    }
    renderer()->setWindow(window);
}

extern "C" JNIEXPORT void JNICALL
Java_org_inkscape_alpha_MainActivity_nativeResize(JNIEnv *, jclass, jint w, jint h)
{
    renderer()->resize(w, h);
}

extern "C" JNIEXPORT void JNICALL
Java_org_inkscape_alpha_MainActivity_nativeFrame(JNIEnv *, jclass)
{
    renderer()->requestFrame();
}

extern "C" JNIEXPORT void JNICALL
Java_org_inkscape_alpha_MainActivity_nativeTouch(JNIEnv *, jclass, jfloat x, jfloat y, jint action)
{
    renderer()->touch(x, y, action);
}

extern "C" JNIEXPORT void JNICALL
Java_org_inkscape_alpha_MainActivity_nativeSetTool(JNIEnv *, jclass, jint tool)
{
    renderer()->setTool(tool);
}