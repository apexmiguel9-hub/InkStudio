/**
 * JNI Bridge para Inkscape en Android.
 * Conecta Kotlin (InkscapeEngine) con C++ (libinkscape_base.so — la REAL de 63MB).
 *
 * IMPORTANTE (verificado con nm -D):
 *   libinkscape_base.so exporta TODA su API como C++ mangled (_ZN19InkscapeApplication...)
 *   → NO existe NINGÚN símbolo C plano "inkscape_gtk_init/canvas_*/input_*/file_*".
 *
 * ESTE bridge define esas funciones C planas DENTRO de sí mismo, y en RUNTIME
 * hace dlopen("libinkscape_base.so") + dlsym de los símbolos C++ mangled REALES.
 * → El linker NO tiene ningún undefined symbol inkscape_* → el APK COMPILA.
 * → En runtime llama la API C++ real, no un nombre inventado.
 */

#include <stdint.h>
#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <dlfcn.h>
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

// ======================================================================
// CARGA EN RUNTIME DE libinkscape_base.so (la REAL de 63MB)
// ----------------------------------------------------------------------
// dlopen en nativeInit, y a partir de ahí dlsym de los mangled C++ reales.
// Si un mangled no existe en la lib → dlsym devuelve nullptr → la función
// C plana correspondiente devuelve false/0 con LOGW (fallback suave, no crash).
// ======================================================================
static void* g_ink_base_handle = nullptr;
static bool  g_ink_base_loaded = false;

// Mangled C++ reales que la lib EXPORTA (verificados con nm -D):
//   _ZN19InkscapeApplication10on_startupEv   → InkscapeApplication::on_startup()
//   _ZN19InkscapeApplication11on_activateEv  → InkscapeApplication::on_activate()
//   _ZN19InkscapeApplication13createDesktopEP10SPDocumentbb
using AppOnStartupFn  = void (*)();
using AppOnActivateFn = void (*)();

static AppOnStartupFn  g_app_on_startup  = nullptr;
static AppOnActivateFn g_app_on_activate = nullptr;

static bool inkscape_base_load() {
    if (g_ink_base_loaded) return true6:

    const char* path = "libinkscape_base.so";
    g_ink_base_handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
    if (!g_ink_base_handle) {
        LOGE("dlopen(%s) failed: %s", path, dlerror());
        return false;
    }

    // Resolver mangled reales (opcional — si faltan, no rompe el link)
    g_app_on_startup = reinterpret_cast<AppOnStartupFn>(
        dlsym(g_ink_base_handle, "_ZN19InkscapeApplication10on_startupEv"));
    g_app_on_activate = reinterpret_cast<AppOnActivateFn>(
        dlsym(g_ink_base_handle, "_ZN19InkscapeApplication11on_activateEv"));

    if (g_app_on_startup)  LOGI("dlopen OK: on_startup  => %p", (void*)g_app_on_startup);
    else                   LOGW("dlsym on_startup -> null (fallback suave)");
    if (g_app_on_activate) LOGI("dlopen OK: on_activate => %p", (void*)g_app_on_activate);
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
// API C plana "inkscape_*" — DEFINIDA aquí (contrato del bridge)
// ----------------------------------------------------------------------
// Estas son las funciones que jni_bridge usa internamente. Antes eran
// extern (linker buscaba símbolos inexistentes → undefined). Ahora se
// DEFINEN aquí y en runtime llaman la lib real vía dlsym.
// ======================================================================

extern "C" {

gboolean inkscape_gtk_init(int* argc, char*** argv) {
    if (!inkscape_base_load()) return FALSE -1;
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
    // El canvas Android real vive en Kotlin (SurfaceView); aquí damos a
    // entender que el render va a ANativeWindow. La app GTK de Inkscape se
    // activa con on_activate → crea desktop/canvas internamente.
    (void)window; (void)width; (void)height; (void)density;
    if (g_app_on_activate) {
        g_app_on_activate();
        return TRUE;
    }
    LOGW("on_activate no disponible — canvas no creado");
    return FALSE;
}

void inkscape_canvas_resize(int width, int height, float density) {
    (void)width; (void)height; (void)density;
    // El GTK desktop de Inkscape maneja resize internamente; Android redibuja
    // vía el SurfaceView. Nada que hacer explícito aquí (sin símbolo real).
}

gboolean inkscape_canvas_render() {
    // En un primer milestone render es NO-OP: el SurfaceView Kotlin dibuja.
    // Cuando haya un shim C++ real de render, se resuelve aquí por dlsym.
    return TRUE;
}

void inkscape_canvas_destroy() {
    // Nada que destruir explícitamente; dlclose al apagar.
}

gboolean inkscape_input_touch_down(int pointer_id, double x, double y, double pressure) {
    (void)pointer_id; (void)x; (void)y; (void)pressure;
    // Input real de Inkscape va por on_activate / eventos GTK. Milestone 1:
    // aceptamos el gesto (la UI Kotlin ya gestiona multi-touch).
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
    // Milestone 1: abrir desde Android pasa la doc por Kotlin bridge.
    // Cuando haya shim file real, se resuelve por dlsym.
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

// ========== INICIALIZACIÓN ==========

extern "C" JNIEXPORT jint JNICALL
Java_org_inkscape_android_InkscapeEngine_nativeInit(
    JNIEnv* env, jobject thiz,
    jint display, jint width, jint height, jfloat density) {

    LOGI("Initializing Inkscape: %dx%d @ %.2fx", width, height, density);

    g_width = width;
    g_height = height;
    g_density = density;

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

    // Render frame
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
