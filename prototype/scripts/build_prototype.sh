#!/usr/bin/env bash
#
# Builds the InkAlpha prototype APK from scratch on a GitHub Actions runner:
#   1. meson cross-build of ThorVG (engines=cpu,gl, static) against the NDK
#   2. CMake build of libinkalpha.so (arm64-v8a)
#   3. Manual Android packaging: aapt2 -> javac -> d8 -> zipalign -> apksigner
#
# Runner prerequisites: Android SDK (build-tools;35.0.0, platforms;android-35,
# ndk;27.x), JDK 17, python3 + meson + ninja, git, curl.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)" # prototype/
WORK="$ROOT/build"

SDK="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-}}"
[ -n "$SDK" ] || { echo "ERROR: ANDROID_SDK_ROOT/ANDROID_HOME not set"; exit 1; }

BT="$SDK/build-tools/35.0.0"
PLATFORM_JAR="$SDK/platforms/android-35/android.jar"
NDK="$(ls -d "$SDK"/ndk/* | tail -1)"
TOOLCHAIN="$NDK/toolchains/llvm/prebuilt/linux-x86_64"
TRIPLE="aarch64-linux-android"
SYSROOT="$TOOLCHAIN/sysroot"

echo "== SDK: $SDK"
echo "== NDK: $NDK"
mkdir -p "$WORK"

# ---- 1. ThorVG (API pinned: v1.1.2 — do not move; renderer.cpp targets it) ----
TVG_SRC="$WORK/thorvg"
TVG_TAG="${THORVG_TAG:-v1.1.2}"
if [ ! -d "$TVG_SRC/.git" ]; then
  echo "== ThorVG tag: $TVG_TAG (override with THORVG_TAG)"
  git clone --depth 1 --branch "$TVG_TAG" https://github.com/thorvg/thorvg "$TVG_SRC"
fi

python3 -m pip install --user --quiet meson ninja
export PATH="$HOME/.local/bin:$PATH"

cat > "$WORK/cross-android-arm64.txt" <<EOF
[binaries]
c = '$TOOLCHAIN/bin/${TRIPLE}26-clang'
cpp = '$TOOLCHAIN/bin/${TRIPLE}26-clang++'
ar = '$TOOLCHAIN/bin/llvm-ar'
ranlib = '$TOOLCHAIN/bin/llvm-ranlib'
ld = '$TOOLCHAIN/bin/ld.lld'
strip = '$TOOLCHAIN/bin/llvm-strip'
pkgconfig = '$ROOT/scripts/ndk_pkg_config.sh'

[properties]
pkg-config-sysroot = '$SYSROOT'

[host_machine]
system = 'android'
cpu_family = 'aarch64'
cpu = 'aarch64'
endian = 'little'
EOF

export NDK_SYSROOT="$SYSROOT"
export NDK_TRIPLE="$TRIPLE"

meson setup "$WORK/thorvg-build" "$TVG_SRC" \
  --cross-file "$WORK/cross-android-arm64.txt" \
  -Dengines=cpu,gl \
  -Ddefault_library=static \
  --prefix "$WORK/thorvg-install"
meson install -C "$WORK/thorvg-build"

THORVG_LIB="$(find "$WORK/thorvg-build" "$WORK/thorvg-install" -name 'libthorvg.a' | head -1)"
[ -n "$THORVG_LIB" ] || { echo "ERROR: libthorvg.a not produced"; exit 1; }
echo "== ThorVG static lib: $THORVG_LIB"

# ---- 2. Native lib ----------------------------------------------------------
cmake -S "$ROOT/cpp" -B "$WORK/cmake" \
  -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-26 \
  -DCMAKE_BUILD_TYPE=Release \
  -DTHORVG_INC="$TVG_SRC/inc" \
  -DTHORVG_LIB="$THORVG_LIB"
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

keytool -genkeypair -keystore "$WORK/debug.keystore" \
  -storepass inkalpha -keypass inkalpha \
  -alias inkalpha -keyalg RSA -keysize 2048 -validity 10000 \
  -dname "CN=InkAlpha" >/dev/null 2>&1 || true

"$BT/apksigner" sign \
  --ks "$WORK/debug.keystore" --ks-key-alias inkalpha \
  --ks-pass pass:inkalpha --key-pass pass:inkalpha \
  --out "$WORK/inkalpha-debug.apk" aligned.apk

echo "== OK: $WORK/inkalpha-debug.apk"
ls -la "$WORK/inkalpha-debug.apk"