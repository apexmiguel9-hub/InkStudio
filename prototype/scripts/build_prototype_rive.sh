#!/usr/bin/env bash
#
# Builds the InkAlpha prototype APK on a GitHub Actions runner — RIVE
# EXPERIMENT variant:
#   1. premake5+ninja cross-build of rive-runtime (rive + rive_pls_renderer,
#      Vulkan backend) against the NDK
#   2. CMake build of libinkalpha.so (arm64-v8a) linking the rive static libs
#      + rive_vk_bootstrap (compiled into our own target)
#   3. Manual Android packaging: aapt2 -> javac -> d8 -> zipalign -> apksigner
#
# Runner prerequisites: Android SDK (build-tools;35.0.0, platforms;android-35,
# ndk;27.2.12479018 — Rive REQUIRES exactly r27c), JDK 17, git, make (premake
# bootstrap), python3 (shader build steps), curl.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)" # prototype/
WORK="$ROOT/build"
RIVE_TAG="${RIVE_TAG:-runtime-v0.1.465}"

SDK="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-}}"
[ -n "$SDK" ] || { echo "ERROR: ANDROID_SDK_ROOT/ANDROID_HOME not set"; exit 1; }
echo "== SDK: $SDK"

# Discovery must NEVER abort the script silently: under `set -euo pipefail`
# a failed `ls`/`grep` inside `VAR="$(...)"` kills the script with exit 2 and
# NO diagnostics (that bit us in CI). Guard every substitution with `|| true`
# so real failures surface via the explicit checks below.
BT="$(ls -d "$SDK"/build-tools/*/ 2>/dev/null | sort -V | tail -1 || true)"
BT="${BT%/}"
PLATFORM_JAR="$(ls "$SDK"/platforms/android-*/android.jar 2>/dev/null | sort -V | tail -1 || true)"

[ -x "$BT/aapt2" ] || { echo "ERROR: no usable build-tools under $SDK/build-tools"; exit 1; }
[ -n "$PLATFORM_JAR" ] || { echo "ERROR: no android.jar under $SDK/platforms"; exit 1; }

# Rive pins the NDK to EXACTLY r27c (27.2.12479018) — hard error in
# rive_build_config.lua. The workflow provisions it when absent. Prefer the
# exact dir; the fallback exists only to produce a clear error message.
if [ -d "$SDK/ndk/27.2.12479018" ]; then
  NDK="$SDK/ndk/27.2.12479018"
else
  NDK="$(ls -d "$SDK"/ndk/*/ 2>/dev/null | sort -V | tail -1 || true)"
  NDK="${NDK%/}"
fi
[ -n "$NDK" ] || { echo "ERROR: no NDK under $SDK/ndk (workflow step should have provisioned r27c)"; exit 1; }
NDK_LONG="$(grep -o 'Pkg.Revision = [0-9.]*' "$NDK/source.properties" 2>/dev/null | awk '{print $3}' || true)"
if [ "$NDK_LONG" != "27.2.12479018" ]; then
  echo "ERROR: Rive requires NDK 27.2.12479018 (r27c); found: $NDK ($NDK_LONG)"
  exit 1
fi
echo "== NDK: $NDK"

TOOLCHAIN="$NDK/toolchains/llvm/prebuilt/linux-x86_64"
TRIPLE="aarch64-linux-android"
SYSROOT="$TOOLCHAIN/sysroot"
mkdir -p "$WORK"

# ---- 1. rive-runtime (pinned tag; do not move) ----------------------------
RIVE_SRC="$WORK/rive-runtime"
if [ ! -d "$RIVE_SRC/.git" ]; then
  echo "== Rive tag: $RIVE_TAG"
  git clone --depth 1 --branch "$RIVE_TAG" https://github.com/rive-app/rive-runtime "$RIVE_SRC"
fi

# premake looks for premake5.lua; the repo ships premake5_v2.lua as the main
# project (build_rive.sh is invoked with cwd = repo root).
if [ ! -e "$RIVE_SRC/premake5.lua" ]; then
  ln -s premake5_v2.lua "$RIVE_SRC/premake5.lua"
fi

# Pre-seed premake5 so build_rive.sh SKIPS its source bootstrap. On Linux it
# would `git clone premake-core && make -f Bootstrap.mak linux` otherwise,
# which needs libuuid-dev (absent on the runner: os_uuid.c fails on
# uuid/uuid.h) and costs ~2 min per run. The official linux binary for the
# pinned tag goes to the exact path build_rive.sh checks:
#   build/dependencies/premake-core/bin/<tag>_release/premake5
# Export RIVE_PREMAKE_TAG so the seed path and build_rive.sh can't diverge.
export RIVE_PREMAKE_TAG="${RIVE_PREMAKE_TAG:-v5.0.0-beta7}"
PREMAKE_INSTALL_DIR="$RIVE_SRC/build/dependencies/premake-core/bin/${RIVE_PREMAKE_TAG}_release"
if [ ! -x "$PREMAKE_INSTALL_DIR/premake5" ]; then
  echo "== Pre-seeding premake5 (${RIVE_PREMAKE_TAG} linux binary)"
  mkdir -p "$PREMAKE_INSTALL_DIR"
  curl -fsSL -o /tmp/premake5.tar.gz \
    "https://github.com/premake/premake-core/releases/download/${RIVE_PREMAKE_TAG}/premake-${RIVE_PREMAKE_TAG#v}-linux.tar.gz"
  tar -xzf /tmp/premake5.tar.gz -C "$PREMAKE_INSTALL_DIR"
  chmod +x "$PREMAKE_INSTALL_DIR/premake5"
fi

# Seed the vendored PLS shader corpus BEFORE premake runs: premake's shader
# step (`make -C src/shaders ... spirv`) needs glslangValidator + spirv-opt,
# which are NOT on the runner. SPIR-V is arch-independent, the corpus matches
# the pinned tag (prototype/vendored/rive_shaders/README.md), and touching
# everything makes the mtime-based Makefile see all targets up to date.
# OUT dir == RIVE_BUILD_OUT/include/generated/shaders (out/android_arm64_release).
SHADER_OUT="$RIVE_SRC/out/android_arm64_release/include/generated/shaders"
if [ ! -d "$SHADER_OUT" ] || [ -z "$(ls -A "$SHADER_OUT" 2>/dev/null)" ]; then
  echo "== Seeding vendored PLS shaders -> $SHADER_OUT"
  mkdir -p "$SHADER_OUT"
  cp -r "$ROOT/vendored/rive_shaders/." "$SHADER_OUT/"
  find "$SHADER_OUT" -exec touch {} +
fi

export ANDROID_NDK="$NDK"
cd "$RIVE_SRC"
# --with_vulkan only (no text/layout/canvas: keeps the lib small and is all
# the low-level RiveRenderer path needs). ninja targets: rive + rive_pls_renderer.
RIVE_PREMAKE_ARGS="--with_vulkan" bash build/build_rive.sh ninja release android arm64 \
  -- rive rive_pls_renderer
cd "$ROOT"

RIVE_BUILD="$RIVE_SRC/out/android_arm64_release"
RIVE_LIB="$(find "$RIVE_BUILD" -name 'librive.a' | head -1)"
RIVE_PLS_LIB="$(find "$RIVE_BUILD" -name 'librive_pls_renderer.a' | head -1)"
[ -n "$RIVE_LIB" ] || { echo "ERROR: librive.a not produced under $RIVE_BUILD"; exit 1; }
[ -n "$RIVE_PLS_LIB" ] || { echo "ERROR: librive_pls_renderer.a not produced under $RIVE_BUILD"; exit 1; }
echo "== Rive libs: $RIVE_LIB / $RIVE_PLS_LIB"

# ---- 2. Native lib (links the rive static libs + our tools/registry) --------
cmake -S "$ROOT/cpp" -B "$WORK/cmake" \
  -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-26 \
  -DANDROID_STL=c++_static \
  -DCMAKE_BUILD_TYPE=Release \
  -DRIVE_ROOT="$RIVE_SRC" \
  -DRIVE_BUILD="$RIVE_BUILD"
cmake --build "$WORK/cmake" --target inkalpha -- -j4

# ---- 3. Package -------------------------------------------------------------
PKG="$WORK/apk"
rm -rf "$PKG"
mkdir -p "$PKG/classes" "$PKG/lib/arm64-v8a"
cp "$WORK/cmake/libinkalpha.so" "$PKG/lib/arm64-v8a/"

"$BT/aapt2" link -o "$PKG/base.apk" \
  --manifest "$ROOT/android/AndroidManifest.xml" \
  -I "$PLATFORM_JAR" \
  --min-sdk-version 26 \
  --target-sdk-version 35

javac -source 11 -target 11 -encoding UTF-8 \
  -classpath "$PLATFORM_JAR" \
  -d "$PKG/classes" \
  $(find "$ROOT/android/java" -name '*.java')

"$BT/d8" --release --lib "$PLATFORM_JAR" --min-api 26 \
  --output "$PKG" $(find "$PKG/classes" -name '*.class')

cd "$PKG"
zip -q -u base.apk classes.dex lib/arm64-v8a/libinkalpha.so

"$BT/zipalign" -f 4 base.apk aligned.apk

KEYSTORE="$ROOT/android/debug.keystore"
if [ ! -f "$KEYSTORE" ]; then
  echo "WARNING: no committed debug.keystore; generating an ad-hoc one (signature will differ per build)"
  KEYSTORE="$WORK/debug.keystore"
  keytool -genkeypair -keystore "$KEYSTORE" \
    -storepass inkalpha -keypass inkalpha \
    -alias inkalpha -keyalg RSA -keysize 2048 -validity 10000 \
    -dname "CN=InkAlpha" >/dev/null 2>&1 || true
fi

"$BT/apksigner" sign \
  --ks "$KEYSTORE" --ks-key-alias inkalpha \
  --ks-pass pass:inkalpha --key-pass pass:inkalpha \
  --out "$WORK/inkalpha-debug.apk" aligned.apk

echo "== OK: $WORK/inkalpha-debug.apk"
ls -la "$WORK/inkalpha-debug.apk"