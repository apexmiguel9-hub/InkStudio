# InkAlpha — prototype nativo Android (NDK + ThorVG, backend GL)

Primera alpha funcional del port Android nativo de Inkscape: canvas fullscreen
renderizado con **ThorVG (GlCanvas)**, y **una** tool real — el **rectángulo** — cuyo
código de interacción está **portado literalmente de `ui/tools/rect-tool.cpp`** de
Inkscape (no reescrito), con adaptación mínima de la capa de servicios.

## Qué valida esta alpha (el objetivo)

- Touch → **los mismos hooks de mouse** que ya usa la tool en Inkscape
  (ButtonPress/Motion/ButtonRelease vía `root_handler`), sin gestos propios.
- El matemático del drag (`snap_rectangular_box`: Ctrl=confine ratio/golden,
  Alt=off-center) **verbatim** de Inkscape, con el motor de snapping stubeado.
- Modelo de objeto: `SPRect::setPosition/setRx/setRy` **verbatim** de sp-rect.cpp.
- El reemplazo real de Cairo→ThorVG ocurre en el **renderer** (registry de
  rects vivos → `tvg::Shape` por frame), no dentro de la tool (que nunca pintó).

## Estructura

```
prototype/
  cpp/
    renderer.{h,cpp}      # ThorVG GlCanvas + síntesis touch->CanvasEvent
    jni_bridge.cpp        # JNI (nativeInit/Resize/Frame/Touch/SetTool)
    tool/rect_tool_port.{h,cpp}   # port de rect-tool.cpp/h (algoritmo verbatim)
    sham/                 # harness mínimo de servicios de Inkscape
      tool_base.*  canvas_events.h  prefs.h  message.h  selection.h
      desktop.*    snap.{h,cpp}     sp_rect.h  mods.h  undo.h  util.h
      geom_min.h   ink_compat.h
  android/
    AndroidManifest.xml
    java/org/inkscape/alpha/MainActivity.java   # Toolbox + GLSurfaceView
  scripts/
    build_prototype.sh    # ThorVG(meson cross) + lib + empaquetado manual
    ndk_pkg_config.sh     # shim pkg-config GLES/EGL para el GL engine
```

## Build (GitHub Actions, repo InkStudio)

El workflow `.github/workflows/build-inkalpha.yml` (rama `alpha-thorvg-rect`)
compila todo desde cero en ubuntu-latest:

1. **ThorVG** **fijado al tag `v1.1.2`** (API confirmada contra su `inc/thorvg.h`;
   `renderer.cpp` está escrito contra esa API), cross para arm64 con el NDK
   (meson, `-Dengines=cpu,gl -Ddefault_library=static`).
2. **libinkalpha.so** vía CMake + `android.toolchain.cmake` (arm64-v8a, minSdk 26).
3. **APK** empaquetado manual (sin Gradle): aapt2 link → javac → d8 → zip de
   `classes.dex` + `.so` → zipalign → apksigner (keystore debug generado).

Artefacto: `inkalpha-debug-apk` → `prototype/build/inkalpha-debug.apk`.

### Local (solo inspección/lógica, sin compilar Android)

El código C++ del prototipo es portable a desktop: el harness (sham/) y la tool
se pueden compilar con un clang/g++ común, y el renderer puede probarse con el
engines **cpu** de ThorVG offscreen. El APK en sí solo se compila en CI por el
NDK.

## Uso en el g56

1. Descargar `inkalpha-debug.apk` del artefacto del workflow.
2. Instalar (permite "instalar apps desconocidas"): `adb install` or tap del .apk.
3. Abrir **InkAlpha** → tocar **Rectángulo** (ya viene seleccionada) → arrastrar
   en el canvas → al soltar el rect se confirma.

## Estado actual / faltantes

- ✅ Rect confirmado en el canvas (se queda dibujado).
- ✅ Preview en vivo mientras arrastras (el mismo SPRect actualizado por motion).
- ✅ Tolerancia de drag (dragtolerance=6px) para que el micro-jitter del dedo no
  cree rects accidentales — desviación touch documentada del default 0 de Inkscape.
- ❌ Mover/redimensionar el rect después de creado (selector + knotholders).
- ❌ Más tools (elical, elipse, path... — misma receta rect-tool).
- ❌ Vulkan (ThorVG wg) — fase posterior, explícitamente fuera de esta alpha.
- ❌ Documento XML real (el bridge `svg:rect` ↔ SPRect) — reemplazará el registry.
- ❌ Undo/redo, snapping real, selección, capas.