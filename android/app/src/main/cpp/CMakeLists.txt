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
set(INKSCAPE_LIBS
    inkscape        # libinkscape.so  (API interna inkscape_gtk_init/canvas/input)
    inkview         # libinkview.so   (inkscape_canvas_render helpers)
    inkscape_base   # libinkscape_base.so (utilidades base internas)
    gtk-4           # libgtk-4.so     (gtk_window/gdk/cairo directos del bridge)
    gdk_pixbuf-2.0  # libgdk_pixbuf-2.0.so (gdk_pixbuf que usa gtk-4 en runtime)
    cairo           # libcairo.so     (cairo_surface_t que usa gdk en runtime)
    cairo-gobject   # libcairo-gobject.so (registro gobject de cairo)
    pango-1.0       # libpango-1.0.so (pango layout usado por gtk-4 en runtime)
    pangocairo-1.0  # libpangocairo-1.0.so
    freetype        # libfreetype.so  (fuentes)
    fribidi         # libfribidi.so
    harfbuzz        # libharfbuzz.so
    glib-2.0        # libglib-2.0.so
    gio-2.0         # libgio-2.0.so
    gobject-2.0     # libgobject-2.0.so
    gmodule-2.0     # libgmodule-2.0.so
    graphene-1.0    # libgraphene-1.0.so
    epoxy           # libepoxy.so
    xml2            # libxml2.so
    z               # libz.so
    dl              # dlopen/dlsym
    log             # __android_log_print
    android         # ANativeWindow
    m               # matematicas
    atomic
)


add_library(inkscape_jni SHARED jni_bridge.cpp)

target_link_libraries(inkscape_jni ${INKSCAPE_LIBS})

# Exportar símbolos JNI
target_compile_definitions(inkscape_jni PRIVATE
    JNI_VERSION=JNI_VERSION_1_6
)