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

# Añadir automáticamente los subdirectorios de include/
# PERO excluyendo dirs que contienen headers con nombre de sistema en su raíz
# (p.ej. textstyle/stdbool.h, manifold/math.h -> shadowean los del NDK/libc++)
file(GLOB _INKSCAPE_INCLUDE_SUBDIRS LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/include/*)
set(_SHADOW_HEADERS stdint.h stdbool.h stdarg.h math.h stdlib.h stdio.h string.h
    limits.h float.h stddef.h stdatomic.h stdalign.h inttypes.h wchar.h ctype.h
    errno.h assert.h stdio.h locale.h setjmp.h signal.h time.h)
foreach(_dir IN LISTS _INKSCAPE_INCLUDE_SUBDIRS)
    if(IS_DIRECTORY "${_dir}")
        set(_skip FALSE)
        foreach(_hdr IN LISTS _SHADOW_HEADERS)
            if(EXISTS "${_dir}/${_hdr}")
                set(_skip TRUE)
                message(STATUS "=== SKIP include dir (shadow header ${_hdr}): ${_dir} ===")
                break()
            endif()
        endforeach()
        if(NOT _skip)
            include_directories("${_dir}")
            message(STATUS "=== Include dir: ${_dir} ===")
        endif()
    endif()
endforeach()

# Fallback: también buscar en jniLibs por si acaso
include_directories(${CMAKE_SOURCE_DIR}/../../../jniLibs/arm64-v8a/include)

# Link libraries (prebuilt .so/.a en jniLibs)
link_directories(${CMAKE_SOURCE_DIR}/../../../jniLibs/arm64-v8a)

# Librerías a linkear (orden de dependencias)
set(INKSCAPE_LIBS
    inkscape
    inkview
    inkscape_base
    gtk-4
    gtkmm-4.0
    pangomm-2.48
    cairomm-1.16
    glibmm-2.68
    giomm-2.68
    sigc-3.0
    cairo
    cairo-gobject
    pango-1.0
    pangocairo-1.0
    pangoft2-1.0
    gdk_pixbuf-2.0
    graphene-1.0
    gobject-2.0
    glib-2.0
    gio-2.0
    gmodule-2.0
    freetype
    harfbuzz
    fribidi
    pixman-1
    epoxy
    xml2
    png16
    jpeg
    z
    lzma
    zstd
    brotlidec
    brotlicommon
    expat
    pcre2-8
    ffi
    intl
    iconv
    lcms2
    dl
    log
    android
    m
    atomic
)

add_library(inkscape_jni SHARED jni_bridge.cpp)

target_link_libraries(inkscape_jni ${INKSCAPE_LIBS})

# Exportar símbolos JNI
target_compile_definitions(inkscape_jni PRIVATE
    JNI_VERSION=JNI_VERSION_1_6
)