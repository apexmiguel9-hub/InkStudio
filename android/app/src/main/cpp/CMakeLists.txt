cmake_minimum_required(VERSION 3.22.1)
project("inkscape_jni" LANGUAGES CXX)

# Configuración Android
if(ANDROID)
    set(CMAKE_ANDROID_NDK ${CMAKE_ANDROID_NDK} CACHE PATH "NDK path")
    set(CMAKE_ANDROID_API 29)
    set(CMAKE_ANDROID_STL_TYPE c++_static)
endif()

# VERIFICACIÓN NDK: imprime versión y ruta que CMake está usando
message(STATUS "=== VERIFICACION NDK: ${CMAKE_ANDROID_NDK} ===")

# C++17 requerido
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include dirs para headers de Inkscape/GTK
# Headers copiados a ${CMAKE_SOURCE_DIR}/include/ (android/app/src/main/cpp/include/)
include_directories(${CMAKE_SOURCE_DIR}/include)

# WHITELIST: solo los dirs de GTK/gtkmm que el bridge necesita.
# NO usar GLOB: blender trae basura (libavcodec/jni.h, python3.13, OSL, ...)
# que shadowea los headers del NDK (jni.h, stdint.h, math.h, ...)
set(_INKSCAPE_INCLUDE_DIRS
    gtk-4.0
    gdk-pixbuf-2.0
    gio-unix-2.0
    glib-2.0
    graphene-1.0
    gtkmm-4.0
    glibmm-2.68
    giomm-2.68
    pangomm-2.48
    cairomm-1.16
    sigc++-3.0
    cairo
    pango-1.0
    harfbuzz
    freetype2
    fribidi
    fontconfig
    epoxy
    libxml2
)
foreach(_dir IN LISTS _INKSCAPE_INCLUDE_DIRS)
    if(EXISTS "${CMAKE_SOURCE_DIR}/include/${_dir}")
        include_directories("${CMAKE_SOURCE_DIR}/include/${_dir}")
        message(STATUS "=== Include dir: ${_dir} ===")
    else()
        message(WARNING "=== FALTA include dir (ignorado): ${_dir} ===")
    endif()
endforeach()

# Fallback: también buscar en jniLibs por si acaso
include_directories(${CMAKE_SOURCE_DIR}/../jniLibs/arm64-v8a/include)

# Link libraries (prebuilt .so/.a en jniLibs)
link_directories(${CMAKE_SOURCE_DIR}/../jniLibs/arm64-v8a)

# Librerías a linkear (orden de dependencias)
# ======================================================================
# INKSCAPE_LIBS: MÍNIMO COMBATIBLE (SOLO lo que el bridge usa real)
# ----------------------------------------------------------------------
# El bridge (jni_bridge.cpp) hace 2 únicos grupos de llamadas:
#   1. inkscape_gtk_init / inkscape_canvas_* / inkscape_input_*  => libinkscape.so
#      (esa .so YA incorpora ESTÁTICAMENTE gtkmm/pangomm/cairomm/glibmm/giomm/
#       sigc/boost, porque build-inkscape linkeó los .a tier2 dentro del .so)
#   2. API GTK4 directa (gtk_window_new, gdk surface, cairo)     => libgtk-4.so
#
# NADA más: NO linkear -lgtkmm-4.0, -lpangomm, -lcairomm, -lglibmm,
# -lgiomm, -lsigc, -lcairomm… (no hay .so exportado; estaría mal de
# todos modos porque son símbolos estáticos dentro de libinkscape.so).
# ======================================================================
# ======================================================================
# INKSCAPE_LIBS: MÍNIMO ABSOLUTO (solo lo que el bridge usa DIRECTAMENTE)
# ----------------------------------------------------------------------
# El bridge (jni_bridge.cpp) tiene EXACTAMENTE estos grupos de símbolos undefined:
#   1. inkscape_gtk_init / inkscape_gtk_shutdown                 => libinkscape.so
#   2. inkscape_canvas_create/resize/render/destroy              => libinkscape.so
#   3. inkscape_input_touch_* / inkscape_input_scroll/scale      => libinkscape.so
#   4. ANativeWindow_* / AHardwareBuffer_*                       => -landroid
#
# NO usa cairo/pango/glib/gobject/gio/epoxy/graphene DIRECTAMENTE:
# esos símbolos los toca libinkscape.so INTERNAMENTE (y su dyld de runtime
# los resuelve contra las .so que se copien a jniLibs — eso lo asegura el
# workflow build-apk paso 3 con git lfs pull, NO el link estático).
#
# IMPORTANTE: el linker de Android resuelve los símbolos UNDEFINED del bridge
# SOLO contra lo que aquí se lista. Cualquier -l "de más" haría falta que
# exista un .so en jniLibs con ese SONAME, y como NO lo linkeamos contra él,
# no debe estar. ¡NO añadir cairo/pango/gtk-4/gdk_pixbuf aquí!
# ======================================================================
# ======================================================================
# INKSCAPE_LIBS: MÍNIMO ABSOLUTO — SOLO lo que jni_bridge.cpp USA REAL
# ======================================================================
# El bridge tiene EXACTAMENTE estos símbolos undefined:
#   inkscape_gtk_init / inkscape_gtk_shutdown / inkscape_canvas_*
#     / inkscape_input_*   =>  libinkscape.so   (API interna, único proveedor)
#   ANativeWindow_lock/unlock/release           =>  -landroid (NDK)
#   __android_log_print                          =>  -llog
#
# NADA MÁS. NO -lgtk-4, NO -lcairo, NO -lpango, NO -lglib, NO -lffmpeg...
# Esas NO las necesita el bridge: son dependencias INTERNAS de libinkscape.so
# que el runtime carga desde jniLibs (DT_NEEDED) — no del link del .so JNI.
#
# Si el linker pidiera cairo/pango/glib sería porque JNI las referencia;
# el bridge NO las referencia → no -l. ¡NO AÑADIR NADA AQUÍ!
# ======================================================================
set(INKSCAPE_LIBS
    inkscape_base   # libinkscape_base.so  (63MB REAL — TODA la API C++ Inkscape)
    gtk-4           # libgtk-4.so          (GTK4 dinámico que el bridge toca)
    android         # ANativeWindow_*      (window nativa)
    log             # __android_log_print
    dl              # dlopen/dlsym
    m               # matemáticas
    atomic
)

add_library(inkscape_jni SHARED jni_bridge.cpp)

target_link_libraries(inkscape_jni ${INKSCAPE_LIBS})

# Exportar símbolos JNI
target_compile_definitions(inkscape_jni PRIVATE
    JNI_VERSION=JNI_VERSION_1_6
)
