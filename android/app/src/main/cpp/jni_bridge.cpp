/**
 * JNI Bridge para Inkscape en Android.
 * Conecta Kotlin (InkscapeEngine) con C++ (libinkscape + GTK4 backend Android).
 */

#include <stdint.h>
#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <cairo/cairo.h>
#include <string>
#include <mutex>
#include <unordered_map>

#define LOG_TAG "InkscapeJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

// Forward declarations de la API interna de Inkscape
// (estos símbolos vienen de libinkscape.so)
extern "C" {
    // Inicialización GTK
    gboolean inkscape_gtk_init(int* argc, char*** argv);
    void inkscape_gtk_shutdown();

    // Canvas/Render
    gboolean inkscape_canvas_create(ANativeWindow* window, int width, int height, float density);
    void inkscape_canvas_resize(int width, int height, float density);
    gboolean inkscape_canvas_render();
    void inkscape_canvas_destroy();

    // Input
    gboolean inkscape_input_touch_down(int pointer_id, double x, double y, double pressure);
    gboolean inkscape_input_touch_move(int pointer_id, double x, double y, double pressure);
    gboolean inkscape_input_touch_up(int pointer_id, double x, double y);
    gboolean inkscape_input_touch_cancel(int pointer_id);
    gboolean inkscape_input_scroll(double dx, double dy);
    gboolean inkscape_input_scale(double factor, double focus_x, double focus_y);

    // Archivos
    gboolean inkscape_file_open(const char* path);
    gboolean inkscape_file_save(const char* path);
    gboolean inkscape_file_new();
}

// Estado global
static std::mutex g_mutex;
static bool g_initialized = false;
static ANativeWindow* g_native_window = nullptr;
static int g_width = 0;
static int g_height = 0;
static float g_density = 1.0f;

// Touch tracking para multi-touch
static std::unordered_map<int, std::pair<double, double>> g_touch_points;

extern "C" JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("JNI_OnLoad");
    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT void JNICALL
JNI_OnUnload(JavaVM* vm, void* reserved) {
    LOGI("JNI_OnUnload");
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_initialized) {
        inkscape_canvas_destroy();
        inkscape_gtk_shutdown();
        g_initialized = false;
    }
    if (g_native_window) {
        ANativeWindow_release(g_native_window);
        g_native_window = nullptr;
    }
}

// ========== INICIALIZACIÓN ==========

extern "C" JNIEXPORT jint JNICALL
Java_org_inkscape_android_InkscapeEngine_nativeInit(
    JNIEnv* env, jobject thiz,
    jlong display, jint width, jint height, jfloat density) {

    std::lock_guard<std::mutex> lock(g_mutex);

    if (g_initialized) {
        LOGW("Already initialized");
        return 0; // INIT_OK
    }

    g_width = width;
    g_height = height;
    g_density = density;

    LOGI("Initializing Inkscape: %dx%d @ %.2fx", width, height, density);

    // Inicializar GTK (sin argc/argv real, pasamos dummy)
    int argc = 1;
    char* argv[] = { const_cast<char*>("inkscape"), nullptr };
    char** argv_ptr = argv;
    if (!inkscape_gtk_init(&argc, &argv_ptr)) {
        LOGE("inkscape_gtk_init failed");
        return -2; // INIT_ERROR_GTK_INIT
    }

    // Obtener ANativeWindow desde la Surface (passed via display o global)
    // Nota: display aquí es 0, usamos la surface que ya tenemos en Kotlin
    // En la práctica, se pasa la Surface desde Kotlin y se convierte aquí
    // Por ahora asumimos que g_native_window se setea externamente
    if (!g_native_window) {
        LOGE("No native window available");
        return -3; // INIT_ERROR_SURFACE
    }

    if (!inkscape_canvas_create(g_native_window, width, height, density)) {
        LOGE("inkscape_canvas_create failed");
        inkscape_gtk_shutdown();
        return -3;
    }

    g_initialized = true;
    LOGI("Inkscape initialized successfully");
    return 0; // INIT_OK
}

extern "C" JNIEXPORT void JNICALL
Java_org_inkscape_android_InkscapeEngine_nativeSetSurface(
    JNIEnv* env, jobject thiz, jobject surface) {

    std::lock_guard<std::mutex> lock(g_mutex);

    if (g_native_window) {
        ANativeWindow_release(g_native_window);
    }

    g_native_window = ANativeWindow_fromSurface(env, surface);
    if (g_native_window) {
        LOGI("Native window acquired: %p", g_native_window);
    } else {
        LOGE("Failed to get native window from surface");
    }
}

extern "C" JNIEXPORT jint JNICALL
Java_org_inkscape_android_InkscapeEngine_nativeResize(
    JNIEnv* env, jobject thiz,
    jint width, jint height, jfloat density) {

    std::lock_guard<std::mutex> lock(g_mutex);

    if (!g_initialized) return -1;

    g_width = width;
    g_height = height;
    g_density = density;

    LOGI("Resize: %dx%d @ %.2fx", width, height, density);
    inkscape_canvas_resize(width, height, density);
    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_org_inkscape_android_InkscapeEngine_nativeRender(
    JNIEnv* env, jobject thiz) {

    if (!g_initialized) return -1;

    // Render frame
    if (!inkscape_canvas_render()) {
        return -1;
    }
    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_org_inkscape_android_InkscapeEngine_nativeShutdown(
    JNIEnv* env, jobject thiz) {

    std::lock_guard<std::mutex> lock(g_mutex);

    if (!g_initialized) return 0;

    inkscape_canvas_destroy();
    inkscape_gtk_shutdown();

    if (g_native_window) {
        ANativeWindow_release(g_native_window);
        g_native_window = nullptr;
    }

    g_initialized = false;
    g_touch_points.clear();
    return 0;
}

// ========== INPUT ==========

extern "C" JNIEXPORT jboolean JNICALL
Java_org_inkscape_android_InkscapeEngine_nativeTouchDown(
    JNIEnv* env, jobject thiz,
    jint pointerId, jfloat x, jfloat y, jfloat pressure) {

    if (!g_initialized) return JNI_FALSE;

    g_touch_points[pointerId] = {static_cast<double>(x), static_cast<double>(y)};
    return inkscape_input_touch_down(pointerId, x, y, pressure) ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_inkscape_android_InkscapeEngine_nativeTouchMove(
    JNIEnv* env, jobject thiz,
    jint pointerId, jfloat x, jfloat y, jfloat pressure) {

    if (!g_initialized) return JNI_FALSE;

    g_touch_points[pointerId] = {static_cast<double>(x), static_cast<double>(y)};
    return inkscape_input_touch_move(pointerId, x, y, pressure) ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_inkscape_android_InkscapeEngine_nativeTouchUp(
    JNIEnv* env, jobject thiz,
    jint pointerId, jfloat x, jfloat y) {

    if (!g_initialized) return JNI_FALSE;

    g_touch_points.erase(pointerId);
    return inkscape_input_touch_up(pointerId, x, y) ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_inkscape_android_InkscapeEngine_nativeTouchCancel(
    JNIEnv* env, jobject thiz, jint pointerId) {

    if (!g_initialized) return JNI_FALSE;

    g_touch_points.erase(pointerId);
    return inkscape_input_touch_cancel(pointerId) ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_inkscape_android_InkscapeEngine_nativeScroll(
    JNIEnv* env, jobject thiz, jfloat dx, jfloat dy) {

    if (!g_initialized) return JNI_FALSE;
    return inkscape_input_scroll(dx, dy) ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_inkscape_android_InkscapeEngine_nativeScale(
    JNIEnv* env, jobject thiz,
    jfloat factor, jfloat focusX, jfloat focusY) {

    if (!g_initialized) return JNI_FALSE;
    return inkscape_input_scale(factor, focusX, focusY) ? JNI_TRUE : JNI_FALSE;
}

// ========== ARCHIVOS ==========

extern "C" JNIEXPORT jboolean JNICALL
Java_org_inkscape_android_InkscapeEngine_nativeOpenFile(
    JNIEnv* env, jobject thiz, jstring path) {

    if (!g_initialized) return JNI_FALSE;

    const char* cpath = env->GetStringUTFChars(path, nullptr);
    jboolean result = inkscape_file_open(cpath) ? JNI_TRUE : JNI_FALSE;
    env->ReleaseStringUTFChars(path, cpath);
    return result;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_inkscape_android_InkscapeEngine_nativeSaveFile(
    JNIEnv* env, jobject thiz, jstring path) {

    if (!g_initialized) return JNI_FALSE;

    const char* cpath = env->GetStringUTFChars(path, nullptr);
    jboolean result = inkscape_file_save(cpath) ? JNI_TRUE : JNI_FALSE;
    env->ReleaseStringUTFChars(path, cpath);
    return result;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_inkscape_android_InkscapeEngine_nativeNewFile(
    JNIEnv* env, jobject thiz) {

    if (!g_initialized) return JNI_FALSE;
    return inkscape_file_new() ? JNI_TRUE : JNI_FALSE;
}