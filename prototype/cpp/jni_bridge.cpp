// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * jni_bridge.cpp — JNI surface exposed to MainActivity.
 *
 * Static-method JNI (no instance state); the Renderer is a process-lifetime
 * singleton so the GL thread always talks to the same canvas/tool state.
 */
#include <jni.h>

#include "renderer.h"

static Renderer *renderer()
{
    static Renderer *r = new Renderer();
    return r;
}

extern "C" JNIEXPORT void JNICALL
Java_org_inkscape_alpha_MainActivity_nativeInit(JNIEnv *, jclass)
{
    (void)renderer();
}

extern "C" JNIEXPORT void JNICALL
Java_org_inkscape_alpha_MainActivity_nativeResize(JNIEnv *, jclass, jint w, jint h)
{
    renderer()->resize(w, h);
}

extern "C" JNIEXPORT void JNICALL
Java_org_inkscape_alpha_MainActivity_nativeFrame(JNIEnv *, jclass)
{
    renderer()->frame();
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