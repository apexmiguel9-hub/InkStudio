# Toolchain de CMake para compilar Inkscape contra Android arm64 (API 31).
#
# Emparejado exacto con el NDK y las libs de la repo hermana:
#   - NDK r30 (zip oficial de Google; en CI lo deja "Setup NDK" en
#     $HOME/android-ndk-r30, mismo patrón/cache key que lib-inkscape-android_arm64)
#   - aarch64-linux-android31 (Android 12): el target que ya usó el backend
#     Android de gtk4 en su build de tier2.
#   - c++_static: las .a de C++ (gtkmm/glibmm/sigc++/cairomm/pangomm) nunca se
#     linkean a solas, así que la elección de STL es SOLO del enlace final —
#     la misma para toda la app y sin mezcla static/shared de libc++.
#
# Los modos FIND_* son BOTH a propósito: nuestras libs son binarios arm64 ya
# construidos que viven en rutas HOST (.libs/, .blender-libs/) y se encuentran
# vía CMAKE_PREFIX_PATH tal cual. Con ONLY, CMake reescribiría cada prefijo
# contra el sysroot del NDK y no encontraría NADA — la misma clase de bug que
# "sys_root" en el cross-file de meson (run #12 de la repo de libs; también
# aplicó PKG_CONFIG_SYSROOT_DIR vacío por el mismo motivo).

set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# --- localizar el NDK (sin depender de la ruta exacta de la versión) ---
# Precedencia: -DCMAKE_ANDROID_NDK (la pasa el workflow => manda en CI) >
# ANDROID_NDK_HOME > glob $HOME/android-ndk-*. OJO run #4: ubuntu-latest
# exporta ANDROID_NDK_HOME a su NDK 27.3 preinstalado y esta rama ganó: el
# port se habría compilado con un NDK distinto al con el que se construyeron
# las libs (r30). Por eso el workflow pasa -DCMAKE_ANDROID_NDK explícito y
# exporta ANDROID_NDK_HOME apuntando al nuestro.
if(NOT CMAKE_ANDROID_NDK)
  if(DEFINED ENV{ANDROID_NDK_HOME} AND IS_DIRECTORY "$ENV{ANDROID_NDK_HOME}/toolchains/llvm")
    set(CMAKE_ANDROID_NDK "$ENV{ANDROID_NDK_HOME}")
  else()
    file(GLOB _ndk_candidates "$ENV{HOME}/android-ndk-*")
    foreach(_d IN LISTS _ndk_candidates)
      if(IS_DIRECTORY "${_d}/toolchains/llvm")
        set(CMAKE_ANDROID_NDK "${_d}")
        break()
      endif()
    endforeach()
  endif()
endif()
if(NOT CMAKE_ANDROID_NDK
   OR NOT EXISTS "${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/clang")
  message(FATAL_ERROR
    "NDK no encontrado: pasa -DCMAKE_ANDROID_NDK=... o exporta ANDROID_NDK_HOME "
    "(en CI el paso 'Setup NDK' lo deja en $HOME/android-ndk-<ver>)")
endif()

set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
set(CMAKE_ANDROID_API 31)
set(CMAKE_ANDROID_STL_TYPE c++_static)

# Red de seguridad de enlace: libs de bionic que la cadena estática deja sin
# resolver según lo que enlace cada binario (m->libm, log->liblog por si algo
# del backend android de gtk4 registra, android->libandroid de ANativeWindow).
# CMake appendea estas libs AL FINAL de la línea de enlace (CMAKE_*_STANDARD_
# LIBRARIES_INIT), que es el único sitio donde un -l estático sirve de algo
# (orden de barrido del linker).
set(CMAKE_C_STANDARD_LIBRARIES_INIT   "-landroid -llog -lm")
set(CMAKE_CXX_STANDARD_LIBRARIES_INIT "-landroid -llog -lm")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER) # programas = del host (msgfmt...)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
