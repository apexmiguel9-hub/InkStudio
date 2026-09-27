#!/usr/bin/env bash
#
# Builds the Skia experiment APK: libskia_app.so (JNI + Ganesh GL) linked
# against a pre-built Skia, packaged with aapt2/javac/d8/zipalign/apksigner.
#
# Prereqs (runner image):
#   SKIA_DIR             -> skia checkout with out/android-arm64/*.a built
#   ANDROID_SDK_ROOT/HOME-> Android SDK (build-tools, platforms;android-35)
#   JDK 17              on PATH (javac/d8/apksigner/keytool)
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"      # skia-experiment/
SKIA="${SKIA_DIR:?set SKIA_DIR to the prebuilt skia checkout}"

SDK="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-}}"
[ -n "$SDK" ] || { echo "ERROR: ANDROID_SDK_ROOT/ANDROID_HOME not set"; exit 1; }

BT="$(ls -d "$SDK"/build-tools/*/ 2>/dev/null | sort -V | tail -1)"; BT="${BT%/}"
[ -x "$BT/aapt2" ] || { echo "ERROR: no usable build-tools under $SDK/build-tools"; exit 1; }
PLATFORM_JAR="$(ls "$SDK"/platforms/android-*/android.jar 2>/dev/null | sort -V | tail -1)"
[ -n "$PLATFORM_JAR" ] || { echo "ERROR: no android.jar under $SDK/platforms"; exit 1; }
NDK="$(ls -d "$SDK"/ndk/* 2>/dev/null | sort -V | tail -1)"
[ -n "$NDK" ] || { echo "ERROR: no NDK under $SDK/ndk"; exit 1; }
echo "== BT: $BT"
echo "== platform jar: $PLATFORM_JAR"
echo "== NDK: $NDK"

TC="$NDK/toolchains/llvm/prebuilt/linux-x86_64"
TRIPLE="aarch64-linux-android"

WORK="$ROOT/build"
rm -rf "$WORK"
mkdir -p "$WORK/classes" "$WORK/lib/arm64-v8a"

# ---- 1. Native lib: JNI + Skia (Ganesh GL) --------------------------------
STATICS="$(ls "$SKIA"/out/android-arm64/lib*.a 2>/dev/null | grep -v sse41 | tr '\n' ' ')"
[ -n "$STATICS" ] || { echo "ERROR: no lib*.a in $SKIA/out/android-arm64 (run gn gen + ninja first)"; exit 1; }
echo "== linking libskia_app.so with statics:"
echo "$STATICS"

"$TC/bin/${TRIPLE}35-clang++" -std=c++17 -fPIC -shared \
  -I "$SKIA" -I "$SKIA/include" \
  "$ROOT/jni/skia_app_jni.cpp" $STATICS \
  -lEGL -lGLESv2 -landroid -llog \
  -static-libstdc++ \
  -Wl,--gc-sections \
  -o "$WORK/lib/arm64-v8a/libskia_app.so"

echo "== UNSTRIPPED =="
ls -lh "$WORK/lib/arm64-v8a/libskia_app.so"
stat -c%s "$WORK/lib/arm64-v8a/libskia_app.so" | awk '{printf "bytes=%d (%.2f MB)\n", $1, $1/1048576}'

"$TC/bin/llvm-strip" --strip-unneeded "$WORK/lib/arm64-v8a/libskia_app.so" \
  -o "$WORK/lib/arm64-v8a/libskia_app.stripped.so"
echo "== STRIPPED (what the APK ships) =="
ls -lh "$WORK/lib/arm64-v8a/libskia_app.stripped.so"
stat -c%s "$WORK/lib/arm64-v8a/libskia_app.stripped.so" | awk '{printf "bytes=%d (%.2f MB)\n", $1, $1/1048576}'
mv -f "$WORK/lib/arm64-v8a/libskia_app.stripped.so" "$WORK/lib/arm64-v8a/libskia_app.so"

# run 11 crashed on-device: UnsatisfiedLinkError "libc++_shared.so not found"
# (-lc++_static did not neutralize the NDK default). Verify DT_NEEDED now.
echo "== DT_NEEDED =="
"$TC/bin/llvm-readelf" -d "$WORK/lib/arm64-v8a/libskia_app.so" | grep NEEDED
if "$TC/bin/llvm-readelf" -d "$WORK/lib/arm64-v8a/libskia_app.so" | grep -q "libc++_shared"; then
  echo "ERROR: libskia_app.so still depends on libc++_shared.so"; exit 1
fi

# sanity: JNI entry points present?
"$TC/bin/llvm-nm" -D "$WORK/lib/arm64-v8a/libskia_app.so" | grep "T Java_org_inkscape_skia" || true

# ---- 2. Package ------------------------------------------------------------
"$BT/aapt2" link -o "$WORK/base.apk" \
  --manifest "$ROOT/android/AndroidManifest.xml" \
  -I "$PLATFORM_JAR" \
  --min-sdk-version 26 \
  --target-sdk-version 35

javac -source 11 -target 11 -encoding UTF-8 \
  -classpath "$PLATFORM_JAR" \
  -d "$WORK/classes" \
  $(find "$ROOT/android/java" -name '*.java')

"$BT/d8" --release --lib "$PLATFORM_JAR" --min-api 26 \
  --output "$WORK" $(find "$WORK/classes" -name '*.class')

cd "$WORK"
zip -q -u base.apk classes.dex lib/arm64-v8a/libskia_app.so
"$BT/zipalign" -f 4 base.apk aligned.apk

# Reuse the committed ThorVG-prototype debug keystore so `adb install -r`
# works across runs with a stable signature. Fall back to an ad-hoc one.
KEYSTORE="$ROOT/../prototype/android/debug.keystore"
if [ ! -f "$KEYSTORE" ]; then
  echo "WARNING: prototype/android/debug.keystore missing; generating ad-hoc store"
  KEYSTORE="$WORK/debug.keystore"
  keytool -genkeypair -keystore "$KEYSTORE" \
    -storepass inkalpha -keypass inkalpha \
    -alias inkalpha -keyalg RSA -keysize 2048 -validity 10000 \
    -dname "CN=SkiaExp" >/dev/null 2>&1 || true
fi

"$BT/apksigner" sign \
  --ks "$KEYSTORE" --ks-key-alias inkalpha \
  --ks-pass pass:inkalpha --key-pass pass:inkalpha \
  --out "$WORK/skia-exp-debug.apk" aligned.apk

echo "== OK: $WORK/skia-exp-debug.apk"
ls -la "$WORK/skia-exp-debug.apk"