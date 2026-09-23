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
#include <glib.h>
#include <signal.h>
#include <sys/stat.h>
#include <cstdlib>
#include <sys/types.h>
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
//   _ZN19InkscapeApplication8instanceEv     = InkscapeApplication::instance() [static singleton getter]
//   _ZN19InkscapeApplication10on_startupEv   = InkscapeApplication::on_startup()   [non-static method]
//   _ZN19InkscapeApplication11on_activateEv  = InkscapeApplication::on_activate()  [non-static method]
//   _ZN19InkscapeApplication13createDesktopEP10SPDocumentbb
using AppInstanceFn   = void* (*)();         // static InkscapeApplication* instance()
using AppOnStartupFn  = void (*)(void*);     // void on_startup()   [non-static, takes 'this']
using AppOnActivateFn = void (*)(void*);     // void on_activate()  [non-static, takes 'this']

static AppInstanceFn   g_app_instance   = nullptr;
static AppOnStartupFn  g_app_on_startup = nullptr;
static AppOnActivateFn g_app_on_activate = nullptr;

// Global dummy instance (used when singleton constructor fails due to SIOF)
static void* g_dummy_app_instance = nullptr;

static bool inkscape_base_load() {
    if (g_ink_base_loaded) return true;

    const char* path = "libinkscape_base.so";
    g_ink_base_handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
    if (!g_ink_base_handle) {
        LOGE("dlopen(%s) failed: %s", path, dlerror());
        return false;
    }

    // Resolver mangled reales
    g_app_instance   = reinterpret_cast<AppInstanceFn>(
        dlsym(g_ink_base_handle, "_ZN19InkscapeApplication8instanceEv"));
    g_app_on_startup = reinterpret_cast<AppOnStartupFn>(
        dlsym(g_ink_base_handle, "_ZN19InkscapeApplication10on_startupEv"));
    g_app_on_activate = reinterpret_cast<AppOnActivateFn>(
        dlsym(g_ink_base_handle, "_ZN19InkscapeApplication11on_activateEv"));

    if (g_app_instance)    LOGI("dlopen OK: instance     = %p", (void*)g_app_instance);
    else                   LOGW("dlsym instance -> null (fallback suave)");
    if (g_app_on_startup)  LOGI("dlopen OK: on_startup   = %p", (void*)g_app_on_startup);
    else                   LOGW("dlsym on_startup -> null (fallback suave)");
    if (g_app_on_activate) LOGI("dlopen OK: on_activate  = %p", (void*)g_app_on_activate);
    else                   LOGW("dlsym on_activate -> null (fallback suave)");

    g_ink_base_loaded = true;
    return true;
}

static void inkscape_base_unload() {
    if (g_ink_base_handle) {
        dlclose(g_ink_base_handle);
        g_ink_base_handle = nullptr;
    }
    g_app_instance = nullptr;
    g_app_on_startup = nullptr;
    g_app_on_activate = nullptr;
    g_ink_base_loaded = false;
}

// ======================================================================
// GLib log handler que NO aborta (Android: logcat en vez de SIGABRT)
// ======================================================================
static void inkscape_android_log_handler(const gchar* log_domain,
                                          GLogLevelFlags log_level,
                                          const gchar* message,
                                          gpointer user_data) {
    (void)user_data;
    int android_level = ANDROID_LOG_DEBUG;
    if (log_level & G_LOG_LEVEL_ERROR) android_level = ANDROID_LOG_ERROR;
    else if (log_level & G_LOG_LEVEL_CRITICAL) android_level = ANDROID_LOG_FATAL;
    else if (log_level & G_LOG_LEVEL_WARNING) android_level = ANDROID_LOG_WARN;
    else if (log_level & G_LOG_LEVEL_INFO) android_level = ANDROID_LOG_INFO;
    else if (log_level & G_LOG_LEVEL_DEBUG) android_level = ANDROID_LOG_DEBUG;
    
    __android_log_print(android_level, "Inkscape-GLib", "[%s] %s", 
                        log_domain ? log_domain : "GLib", message ? message : "(null)");
    // IMPORTANTE: NO llamar abort() ni g_error() aquí
}

// ======================================================================
// SIGABRT handler: último recurso si GLib escapa a nuestros handlers
// ======================================================================
static void inkscape_sigabrt_handler(int sig) {
    (void)sig;
    __android_log_print(ANDROID_LOG_FATAL, "InkscapeJNI", 
                        "SIGABRT caught - GLib abort escaped handlers! Continuing...");
    // NO llamar a abort() ni _exit() - intentamos seguir vivos
    // Nota: esto es peligroso pero en Android SIGABRT = crash seguro si no lo atrapamos
}

// ======================================================================
// Instala todos los handlers anti-abort (GLib + señal) - llamar TEMPRANO
// ======================================================================
static void install_anti_abort_handlers() {
    // 1) Desactivar TODOS los niveles fatales de GLib
    g_log_set_always_fatal((GLogLevelFlags)0);
    
    // 2) Handler por defecto (dominio NULL)
    g_log_set_default_handler(inkscape_android_log_handler, NULL);
    
    // 3) Handlers para dominios conocidos de GLib/GTK/Inkscape
    static const char* all_domains[] = {
        NULL, "GLib", "Gtk", "Gdk", "Gio", "GModule", "GObject", 
        "Inkscape", "InkscapeApplication", "Preferences", "AutoSave",
        "GC", "Cairo", "Pango", "Fontconfig", "SP", "SPRepr"
    };
    for (const char* dom : all_domains) {
        g_log_set_handler(dom, (GLogLevelFlags)(G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION),
                          inkscape_android_log_handler, NULL);
    }
    
    // 4) Re-forzar always_fatal=0 por si Inkscape lo cambió después
    g_log_set_always_fatal((GLogLevelFlags)0);
    
    // 5) SIGABRT signal handler como red de seguridad FINAL
    struct sigaction sa = {};
    sa.sa_handler = inkscape_sigabrt_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_ONSTACK;
    sigaction(SIGABRT, &sa, NULL);
    
    LOGI("Anti-abort handlers installed (GLib + SIGABRT)");
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
    
    // Handlers ya instalados en JNI_OnLoad; solo re-forzar always_fatal=0
    g_log_set_always_fatal((GLogLevelFlags)0);
    
    // HOME para prefs (si no seteado antes)
    if (!getenv("HOME")) {
        setenv("HOME", "/data/data/org.inkscape.android/files", 1);
        LOGI("HOME set for Inkscape prefs");
    }

    // 2) Configurar variables XDG/GTK obligatorias para que GTK no falle al buscar temas/prefs
    if (!getenv("XDG_DATA_DIRS")) {
        setenv("XDG_DATA_DIRS", "/data/data/org.inkscape.android/files/share:/usr/share", 1);
    }
    if (!getenv("XDG_CONFIG_HOME")) {
        setenv("XDG_CONFIG_HOME", "/data/data/org.inkscape.android/files/config", 1);
    }
    if (!getenv("XDG_CACHE_HOME")) {
        setenv("XDG_CACHE_HOME", "/data/data/org.inkscape.android/files/cache", 1);
    }
    if (!getenv("GTK_DATA_PREFIX")) {
        setenv("GTK_DATA_PREFIX", "/data/data/org.inkscape.android/files", 1);
    }
    LOGI("XDG/GTK env vars set");

    // 3) Verificar/crear directorios mínimos que GTK/Inkscape esperan
    mkdir("/data/data/org.inkscape.android/files/share", 0700);
    mkdir("/data/data/org.inkscape.android/files/config", 0700);
    mkdir("/data/data/org.inkscape.android/files/cache", 0700);
    mkdir("/data/data/org.inkscape.android/files/share/inkscape", 0700);
    mkdir("/data/data/org.inkscape.android/files/share/icons", 0700);
    LOGI("XDG dirs created");

    // 4) Inicializar GC de Inkscape ANTES de on_startup()
    typedef void (*GC_Core_Init_Fn)();
    GC_Core_Init_Fn gc_core_init = reinterpret_cast<GC_Core_Init_Fn>(
        dlsym(g_ink_base_handle, "_ZN8Inkscape2GC4Core4initEv"));
    if (gc_core_init) {
        LOGI("Calling Inkscape::GC::Core::init()...");
        gc_core_init();
        LOGI("Inkscape::GC::Core::init() returned OK");
    } else {
        LOGW("Inkscape::GC::Core::init symbol not found (continuing anyway)");
    }

    // 5) VERIFICACIÓN CRÍTICA: GDK Display debe estar disponible antes de on_startup()
    // Cargamos símbolo gdk_display_get_default desde libgdk-4.so (vía libinkscape_base)
    typedef void* (*GdkDisplayGetDefaultFn)();
    GdkDisplayGetDefaultFn gdk_display_get_default = reinterpret_cast<GdkDisplayGetDefaultFn>(
        dlsym(g_ink_base_handle, "gdk_display_get_default"));
    
    void* display = nullptr;
    if (gdk_display_get_default) {
        display = gdk_display_get_default();
    }
    LOGI("gdk_display_get_default() = %p", display);
    
    // Log de variables críticas para debug
    LOGI("ENV check: HOME=%s, XDG_DATA_DIRS=%s, XDG_CONFIG_HOME=%s", 
         getenv("HOME") ?: "NULL", getenv("XDG_DATA_DIRS") ?: "NULL", getenv("XDG_CONFIG_HOME") ?: "NULL");

    // 6) Si display es NULL, intentar inicializar GDK explícitamente antes de on_startup
    if (!display) {
        LOGW("GDK display is NULL (esperado sin RuntimeApplication/ToplevelActivity glue). "
             "Continuando sin gtk_init() explícito - on_startup() manejará lo necesario.");
    }

    // 7) Obtener instancia singleton de InkscapeApplication
    LOGI("DEBUG: g_app_instance ptr = %p", (void*)g_app_instance);
    void* app_instance = nullptr;
    if (g_app_instance) {
        LOGI("DEBUG: Calling instance()...");
        app_instance = g_app_instance();
        LOGI("InkscapeApplication::instance() returned: %p", app_instance);
    } else {
        LOGE("instance() function not available!");
        return FALSE;
    }
    
    if (!app_instance) {
        LOGE("InkscapeApplication::instance() returned NULL! Singleton not initialized.");
        LOGE("DEBUG: This means constructor didn't run or didn't set static pointer.");
        
        // Constructor crashes due to SIOF (reorder not fully fixing it).
        // Allocate dummy zeroed instance instead.
        LOGW("Skipping constructor (crashes due to SIOF), allocating dummy instance (malloc + zero)...");
        const size_t dummy_size = 4096;
        app_instance = malloc(dummy_size);
        if (app_instance) {
            memset(app_instance, 0, dummy_size);
            LOGI("Allocated dummy InkscapeApplication instance at %p (size=%zu)", app_instance, dummy_size);
            // Store globally for later use in inkscape_canvas_create
            g_dummy_app_instance = app_instance;
        } else {
            LOGE("Failed to allocate dummy instance!");
        }
        
        if (!app_instance) {
            LOGE("Failed to get InkscapeApplication instance!");
            return FALSE;
        }
    }
    
    LOGI("Got InkscapeApplication instance: %p", app_instance);

    // Call on_startup() on dummy instance to initialize Inkscape internals
    // (desktop, canvas, preferences, etc.). Anti-abort handlers should catch any crashes.
    if (g_app_on_startup && app_instance) {
        LOGI("Calling on_startup() on dummy instance...");
        g_app_on_startup(app_instance);
        LOGI("on_startup() returned OK");
    } else {
        LOGW("Skipping on_startup() - function or instance not available");
    }
    
    return TRUE;
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
        // Use dummy instance (created in inkscape_gtk_init) since singleton constructor crashes (SIOF)
        void* app_instance = g_dummy_app_instance;
        if (app_instance) {
            LOGI("InkscapeApplication dummy instance = %p, calling on_activate()...", app_instance);
            g_app_on_activate(app_instance);
            return TRUE;
        } else {
            LOGE("No InkscapeApplication instance available (dummy not created)!");
            return FALSE;
        }
    }
    LOGE("g_app_on_activate is null!");
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
        if (g_app_on_startup && g_app_instance) {
            void* app_instance = g_app_instance();
            if (app_instance) g_app_on_startup(app_instance);
        }
    }
}

extern "C" JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("JNI_OnLoad");
    
    // Instalar handlers anti-abort LO MÁS TEMPRANO POSIBLE
    // (antes de cualquier código de Inkscape que pueda loguear fatal)
    inkscape_base_load();  // carga libinkscape_base.so para acceder a glib
    install_anti_abort_handlers();
    
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
    jlong window, jint width, jint height, jfloat density) {

    std::lock_guard<std::mutex> lock(g_mutex);

    if (g_initialized) {
        LOGW("Already initialized");
        return 0; // INIT_OK
    }

    g_width = width;
    g_height = height;
    g_density = density;

    LOGI("Initializing Inkscape: %dx%d @ %.2fx", width, height, density);

    // Set the native window from the passed window pointer
    g_native_window = reinterpret_cast<ANativeWindow*>(window);
    if (!g_native_window) {
        LOGE("No native window provided");
        return -3; // INIT_ERROR_SURFACE
    }

    LOGI("Using provided native window: %p", g_native_window);

    // Inicializar GTK (sin argc/argv real, pasamos dummy)
    int argc = 1;
    char* argv[] = { const_cast<char*>("inkscape"), nullptr };
    char** argv_ptr = argv;
    if (!inkscape_gtk_init(&argc, &argv_ptr)) {
        LOGE("inkscape_gtk_init failed");
        return -2; // INIT_ERROR_GTK_INIT
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

extern "C" JNIEXPORT jlong JNICALL
Java_org_inkscape_android_InkscapeEngine_nativeGetNativeWindow(
    JNIEnv* env, jobject thiz, jobject surface) {

    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (window) {
        LOGI("Got native window from surface: %p", window);
    } else {
        LOGE("Failed to get native window from surface");
    }
    return reinterpret_cast<jlong>(window);
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
Java_org_inkscape_android_InkscapeEngine_nativeNewDocument(
    JNIEnv* env, jobject thiz) {

    if (!g_initialized) return JNI_FALSE;
    return inkscape_file_new() ? JNI_TRUE : JNI_FALSE;
}
