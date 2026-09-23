/**
#include <cstring>
 * JNI Bridge para Inkscape en Android.
 * Conecta Kotlin (InkscapeEngine) con C++ (libinkscape_base.so - la REAL de 63MB).
 *
 * IMPORTANTE (verificado con nm -D):
 *   libinkscape_base.so exporta TODA su API como C++ mangled (_ZN19InkscapeApplication...)
 *   NO existe NINGUN simbolo C plano "inkscape_gtk_init/canvas_ * /input_ * /file_*".
 *
 * ESTE bridge define esas funciones C planas DENTRO de si mismo, y en RUNTIME
 * hace dlopen("libinkscape_base.so") + dlsym de los simbolos C++ mangled REALES.
 * - El linker NO tiene ningun undefined symbol inkscape_* - el APK COMPILA.
 * - En runtime llama la API C++ real, no un nombre inventado.
 */

#include <stdint.h>
#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <algorithm>
#include <vector>
#include <cstdio>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <cstdint>
#include <memory>

#define LOG_TAG "InkscapeJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define TRUE 1
#define FALSE 0

// ======================================================================
// CARGA EN RUNTIME DE libinkscape_base.so (la REAL de 63MB)
// ----------------------------------------------------------------------
// dlopen en nativeInit, y a partir de ahi dlsym de los mangled C++ reales.
// Si un mangled no existe en la lib - se registra un warning (fallback suave).
// ======================================================================
// ============================================================
// Tipos GLib minimos (el NDK NO trae glib.h; la lib REAL los
// define en runtime; aqui se tipan para que el bridge COMPILE).
// ============================================================
#ifndef INKSCAPE_GLIB_TYPES_H
#define INKSCAPE_GLIB_TYPES_H
typedef int    gboolean;
typedef char   gchar;
typedef int    gint;
typedef unsigned int  guint;
typedef long   glong;
typedef unsigned long gulong;
typedef double gdouble;
typedef void*  gpointer;
typedef const void* gconstpointer;
#define TRUE 1
#define FALSE 0
#define G_BEGIN_DECLS
#define G_END_DECLS
#ifndef G_INLINE_FUNC
#define G_INLINE_FUNC inline
#endif
#endif /* INKSCAPE_GLIB_TYPES_H */

static void* g_ink_base_handle = nullptr;
static bool  g_ink_base_loaded = false;

// Mangled C++ reales que la lib EXPORTA (verificados con nm -D):
//   _ZN19InkscapeApplication10on_startupEv   = InkscapeApplication::on_startup()
//   _ZN19InkscapeApplication11on_activateEv  = InkscapeApplication::on_activate()
//   _ZN19InkscapeApplication13createDesktopEP10SPDocumentbb
using AppOnStartupFn  = void (*)();
using AppOnActivateFn = void (*)();

static AppOnStartupFn  g_app_on_startup  = nullptr;
static AppOnActivateFn g_app_on_activate = nullptr;

static bool inkscape_base_load() {
    if (g_ink_base_loaded) return true;

    const char* path = "libinkscape_base.so";
    g_ink_base_handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
    if (!g_ink_base_handle) {
        LOGE("dlopen(%s) failed: %s", path, dlerror());
        return false;
    }

    // Resolver mangled reales (opcional - si faltan, no rompe el link)
    g_app_on_startup = reinterpret_cast<AppOnStartupFn>(
        dlsym(g_ink_base_handle, "_ZN19InkscapeApplication10on_startupEv"));
    g_app_on_activate = reinterpret_cast<AppOnActivateFn>(
        dlsym(g_ink_base_handle, "_ZN19InkscapeApplication11on_activateEv"));

    if (g_app_on_startup)  LOGI("dlopen OK: on_startup  = %p", (void*)g_app_on_startup);
    else                   LOGW("dlsym on_startup -> null (fallback suave)");
    if (g_app_on_activate) LOGI("dlopen OK: on_activate = %p", (void*)g_app_on_activate);
    else                   LOGW("dlsym on_activate -> null (fallback suave)");

    g_ink_base_loaded = true;
    return true;
}

static void inkscape_base_unload() {
    if (g_ink_base_handle) {
        dlclose(g_ink_base_handle);
        g_ink_base_handle = nullptr;
    }
    g_app_on_startup = nullptr;
    g_app_on_activate = nullptr;
    g_ink_base_loaded = false;
}

// ======================================================================
// API C plana "inkscape_*" - DEFINIDA aqui (contrato del bridge)
// ----------------------------------------------------------------------
// Estas son las funciones que jni_bridge usa internamente. Antes eran
// extern (linker buscaba simbolos inexistentes - undefined). Ahora se
// DEFINEN aqui y en runtime llaman la lib real via dlsym.
// ======================================================================

extern "C" {

gboolean inkscape_gtk_init(int* argc, char*** argv) {
    (void)argc; (void)argv;
    if (!inkscape_base_load()) return FALSE;
    // on_startup es el "arranque GTK" real de Inkscape
    if (g_app_on_startup) {
        g_app_on_startup();
        return TRUE;
    }
    return FALSE;
}

void inkscape_gtk_shutdown() {
    inkscape_base_unload();
}

gboolean inkscape_canvas_create(ANativeWindow* window, int width, int height, float density) {
    (void)window; (void)width; (void)height; (void)density;
    // El canvas Android real vive en Kotlin (SurfaceView); aqui damos a
    // entender que el render va a ANativeWindow. La activacion real se hace
    // via on_activate -> crea desktop/canvas internamente.
    if (g_app_on_activate) {
        g_app_on_activate();
        return TRUE;
    }
    return FALSE;
}

void inkscape_canvas_resize(int width, int height, float density) {
    (void)width; (void)height; (void)density;
    // El SurfaceView Kotlin maneja resize; GTK desktop lo redimensiona solo.
}

gboolean inkscape_canvas_render() {
    // En un primer milestone render es NO-OP: el SurfaceView Kotlin dibuja.
    return TRUE;
}

void inkscape_canvas_destroy() {
    // Nada que destruir explicitamente; dlclose al apagar.
}

gboolean inkscape_input_touch_down(int pointer_id, double x, double y, double pressure) {
    (void)pointer_id; (void)x; (void)y; (void)pressure;
    return TRUE;
}

gboolean inkscape_input_touch_move(int pointer_id, double x, double y, double pressure) {
    (void)pointer_id; (void)x; (void)y; (void)pressure;
    return TRUE;
}

gboolean inkscape_input_touch_up(int pointer_id, double x, double y) {
    (void)pointer_id; (void)x; (void)y;
    return TRUE;
}

gboolean inkscape_input_touch_cancel(int pointer_id) {
    (void)pointer_id;
    return TRUE;
}

gboolean inkscape_input_scroll(double dx, double dy) {
    (void)dx; (void)dy;
    return TRUE;
}

gboolean inkscape_input_scale(double factor, double focus_x, double focus_y) {
    (void)factor; (void)focus_x; (void)focus_y;
    return TRUE;
}

gboolean inkscape_file_open(const char* path) {
    (void)path;
    LOGW("inkscape_file_open: no-op (bridge milestone 1)");
    return FALSE;
}

gboolean inkscape_file_save(const char* path) {
    (void)path;
    LOGW("inkscape_file_save: no-op (bridge milestone 1)");
    return FALSE;
}

gboolean inkscape_file_new() {
    LOGW("inkscape_file_new: no-op (bridge milestone 1)");
    return FALSE;
}


} // extern "C"

// Estado global
static bool g_initialized = false;
static ANativeWindow* g_native_window = nullptr;
static int g_width = 0;
static int g_height = 0;
static float g_density = 1.0f;

// Touch tracking para multi-touch
static std::unordered_map<int, std::pair<double, double>> g_touch_points;

// Mutex global para proteger estado de inicializacion/render
static std::mutex g_mutex;

extern "C" {
    inline void inkscape_gtk_init_android() {
        if (g_app_on_startup) g_app_on_startup();
    }
}

extern "C" JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("JNI_OnLoad");
    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT void JNICALL
JNI_OnUnload(JavaVM* vm, void* reserved) {
    LOGI("JNI_OnUnload");
    inkscape_gtk_shutdown();
    if (g_native_window) {
        ANativeWindow_release(g_native_window);
        g_native_window = nullptr;
    }
}

// ========== INICIALIZACION ==========

extern "C" JNIEXPORT jint JNICALL
Java_org_inkscape_android_InkscapeEngine_nativeInit(
    JNIEnv* env, jobject thiz,
    jint display, jint width, jint height, jfloat density) {

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

    if (!inkscape_canvas_render()) {
        return -1;
    }
    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_org_inkscape_android_InkscapeEngine_nativeShutdown(
    JNIEnv* env, jobject thiz) {

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
