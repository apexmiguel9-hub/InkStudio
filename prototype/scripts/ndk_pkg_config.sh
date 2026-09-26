#!/usr/bin/env bash
#
# Minimal pkg-config shim for cross-compiling ThorVG's GL engine with the
# Android NDK (the NDK ships no pkg-config metadata for GLES/EGL). Any module
# requested is reported as installed and resolves to the NDK sysroot.
#
# Used by meson through the `pkgconfig` entry of the cross file. The NDK
# paths come from the NDK_SYSROOT / NDK_TRIPLE env vars set by
# build_prototype.sh.
set -u
: "${NDK_SYSROOT:?NDK_SYSROOT not set}"
: "${NDK_TRIPLE:?NDK_TRIPLE not set}"

case "${1:-}" in
  --exists|--atleast-pkgconfig-version|--atleast-version|--exact-version|--max-version)
    exit 0 ;;
  --modversion)
    echo "1.0"; exit 0 ;;
  --cflags)
    echo "-I${NDK_SYSROOT}/usr/include"; exit 0 ;;
  --libs)
    echo "-L${NDK_SYSROOT}/usr/lib/${NDK_TRIPLE} -lGLESv2 -lEGL"; exit 0 ;;
  --variable=*)
    exit 0 ;;
  *)
    exit 0 ;;
esac