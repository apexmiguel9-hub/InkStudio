# INKSCAPE_PROGRESS.md

Progreso del port de Inkscape a Android nativo (sin GTK4/Cairo). Build vía CI
(GitHub Actions, repo `apexmiguel9-hub/InkStudio`); fuentes de Inkscape se
usan como referencia/port local (`/root/inkscape`).

Repos: código del prototipo en `prototype/` (rama `alpha-thorvg-rect`).
Estado general: **Alpha 1 funcional (rectángulo)**.

---

## ALPHA 1 — Canvas nativo + tool rectángulo (ThorVG GL) — 2026-09-26

**Objetivo cumplido:** APK instalable con canvas fullscreen (ThorVG GlCanvas,
NDK C++), UNA tool real (rectángulo: tap+drag → preview en vivo → confirmado
al soltar), toolbox mínima reutilizable, sin pan/zoom/multitouch, gesto
aislado entre toolbar y canvas (lección FASE13: vistas separadas).

### Tiempo real del proceso (para calibrar estimaciones)

| Etapa | Tiempo real (sesión) | Notas |
|---|---|---|
| Inventario previo (rect-tool, sp-rect, eventos, snap) | ~45 min | análisis honesto antes de tocar código |
| Port C++ + harness (sham/) + primer build lógico local | ~60 min | incl. 2 rondas de fix por `-fsyntax-only` local (dispatcher inspect_event, tipos geom_min, API ThorVG real) |
| Renderer ThorVG + JNI + MainActivity + packaging script + workflow CI | ~45 min | contra header ThorVG v1.1.2 verificado, NO adivinado |
| APK instalable + depuración en dispositivo g56 (crash SIGSEGV + 3 ciclos de canvas negro → gris+rects) | ~60 min | 4 iteraciones CI (runs 4-7) con verificación por píxeles; ver tabla abajo |
| **Total** | **~3.5 h** | incl. el debugging GL en hardware real (la parte que más cuesta) |

**Qué tan directo fue el port (números honestos):**

- `ui/tools/rect-tool.cpp` (413 líneas): **~87% copy-paste literal**. El 100%
  del algoritmo (root_handler ButtonPress/Motion/ButtonRelease/Key, drag(),
  finishItem(), cancel()) está copiado verbatim. Cambios reales: 8 includes
  redirigidos al harness, bloque de conexión de selección del constructor
  (2 líneas, sigc) y bloque de creación vía nodo XML en drag() (~13 líneas,
  `svg:rect` XML → creación directa del SPRect; el bridge XML llega con el
  documento real).
- **Inventario Cairo en rect-tool.cpp: 0 llamadas.** La tool nunca pintó; el
  reemplazo Cairo→ThorVG vive en el renderer (registry de SPRects → tvg::Shape
  por frame), no dentro de la tool.
- `object/sp-rect.cpp`: **100% agnóstico del motor** (0 referencias Cairo).
  Copiados verbatim `setPosition`/`setRx`/`setRy` + campos; `requestDisplayUpdate`
  = no-op (en Inkscape programa el redibujo del arena/Cairo).
- `snap_rectangular_box` (context-fns.cpp): **100% verbatim**, con el snap
  manager stubeado (`getSnapped()=false`) → geometría pura del drag
  (Ctrl=confine ratio entero/golden, Alt=off-center) ya lista para teclado.
- **Adaptación real = harness de servicios de Inkscape (~700 líneas de stubs
  finos, sham/)**: ToolBase (saveDragOrigin/checkDragMoved verbatim),
  CanvasEvent + inspect_event, Preferences, MessageContext, Selection,
  DocumentUndo, Modifiers, snapping, desktop, geom_min (Point/Rect/Affine).
- `renderer.cpp`/JNI/MainActivity/toolbox/packaging: código nuevo (~450 líneas).

### Detalles de implementación

- Touch → los mismos hooks de la tool (ButtonPress/Motion/ButtonRelease,
  `GDK_BUTTON1_MASK` en motion = drag). Sin gestos propios.
- **dragtolerance = 6px** (pref `/options/dragtolerance/value` sembrada en el
  sham): Inkscape usa 0 con mouse; en touch evita micro-rects por jitter del
  dedo. Desviación documentada y tunable.
- Documento del alpha = registry de SPRects vivos; el rect de la tool es el
  preview (se actualiza por motion) y al soltar `finishItem` lo confirma
  (queda en el registry). `cancel()` (Escape/0-size) lo borra.
- ThorVG **v1.1.2 fijado** (no "latest"): API verificada contra su
  `inc/thorvg.h` (add/remove, GlCanvas::target EGL, strokeWidth, init(0)).
- Build CI: todo en CI, nada local — workflow `build-inkalpha` (dispatch + push a
  la rama), NDK r27, meson cross arm64 (engines cpu,gl), CMake, empaquetado
  manual sin Gradle (aapt2 → javac → d8 → zipalign → apksigner debug).
- Gesture isolation: toolbar y canvas son vistas separadas.

### Iteraciones reales de CI (calibración de estimación)

| Run | Fallo | Fix | Coste |
|---|---|---|---|
| 1 (dispatch 00:28Z) | App Android SDK setup: `android-actions/setup-android@v3` (su `sdkmanager --licenses` interno sale 1 en cmdline-tools 16) | Quitada la action; SDK preinstalado del runner + sdkmanager best-effort; script descubre build-tools/platforms/NDK en vez de rutas fijas | ~5 min |
| 2 (push 00:32Z) | `libthorvg.a not produced`: meson genera `libthorvg-<vmaj>.a` | `find -name 'libthorvg*.a'` | ~3 min |
| 3 (push 00:33Z) | **VERDE** — APK `inkalpha-debug.apk` (1.86 MB) con `libinkalpha.so` arm64 (5 funciones JNI verificadas con readelf) | — | ~2.5 min |

Total CI real hasta APK instalable: **~7 min en 3 iteraciones**. Bugs de
pipeline detectados y corregidos: 2 (setup-android deprecated/roto, nombre de
lib meson). El bug que se evitó de diseño: `-Dextra=opengl_es` obligatorio en
ThorVG (default compila desktop GL — 100% roto en drivers Android).

### Depuración en dispositivo g56 (la parte que nadie estima bien)

El APK instalado NO renderizaba al primer intento. Datos reales (cada fila =
una iteración CI completa + instalación + verificación por píxeles):

| Run | Síntoma en el g56 | Evidencia | Causa raíz | Fix |
|---|---|---|---|---|
| 3 | SIGSEGV al primer frame | logcat `Fatal signal 11` + readelf: símbolos `gl*` = OBJECT local en data (8 bytes), fuera del LOAD RX → `bl` a memoria sin mapear | Nuestro código llamaba GL directo | Cero GL directo en el app; fondo = `tvg::Shape` gris a lienzo completo |
| 4 | Corría, canvas negro | píxeles: gris≈8/162000 muestras (fondo nunca dibujado) | `target()` sin `update()` → escena sin procesar | target + update juntos |
| 5 | Corría, canvas negro | logcat `frame: update=2 draw=2` = `Result::InsufficientCondition` | GlCanvas SIN `target()`: update/draw no pueden renderizar | re-añadir `target(EGL, id=0)` por frame (contexto recreable) |
| 6 | Corría, canvas negro | logcat `update=2 draw=2` aun CON target | `GlRenderer::target` v1.1.2: `if (cs != ColorSpace::ABGR8888S) return NonSupport` (5) — se pasaba `ABGR8888` (premultiplicado) | `ColorSpace::ABGR8888S` |
| 7 | **VERDE** | píxeles: `(239,239,244)` gris canvas, `(52,102,255)` fill rect (blend exacto alpha 240 sobre gris), `(29,42,89)` stroke, toolbar `(30,31,38)` bajo la barra de sistema | — | — |

Lecciones de calibración:
- **4 iteraciones de CI solo para que el render GL funcionara en Android**
  (runs 4-7) pese a tener el header ThorVG real y la impl leída del tag
  v1.1.2 — el header no captura que GlCanvas exige `target()` + `ABGR8888S`.
  Próximo backend: leer `tvgGlRenderer.cpp::target()` del tag ANTES de
  escribir el primer `frame()`.
- Loguear SIEMPRE el Result de `target()` también (no solo update/draw):
  el NonSupport del colorspace era silencioso, solo se veía el 2 colateral
  de update/draw. Enum: Success=0, InvalidArguments=1, InsufficientCondition=2,
  FailedAllocation=3, MemoryCorruption=4, NonSupport=5.
- Pantalla negra ≠ crash: la verificación por píxeles (screencap + muestreo)
  distingue "no hubo draw" de "draw mal"; sin ella, la hipótesis "GLSurfaceView
  no funciona" habría causado rewrites inútiles.
- Firma estable entre builds = keystore debug commiteado → `adb install -r`
  directo sin uninstall, y 1 solo download APK por iteración.

### Validación del APK (verificada, no asumida)

- Estructura: AndroidManifest binario, classes.dex, `lib/arm64-v8a/libinkalpha.so`
  (6.3 MB, ThorVG estático dentro), bloque firma v1 ✓.
- `.so`: ELF64 AArch64 DYN, exports `Java_org_inkscape_alpha_MainActivity_nativeInit/Resize/Frame/Touch/SetTool` ✓.
- **Dispositivo g56 (verificado en vivo)**: instalado con `adb install -r`;
  3 rects dibujados por swipe (`input swipe`) — touch → ButtonPress/Motion/
  ButtonRelease verbatim de rect-tool.cpp — acumulados en el registry
  (verificado por incremento de píxeles azules por cada rect, incluido uno
  disjunto en zona vacía). 0 crashes (`Fatal signal`/SIGSEGV = 0).
  Toolbar visible bajo la barra de sistema/notch (`fitsSystemWindows` —
  fix del reporte "el botón está muy arriba").

### Validación local (sin NDK)

- Port + harness: `clang++ -std=c++17 -fsyntax-only` en los 4 TU → OK.
- renderer.cpp: `-fsyntax-only` contra el header v1.1.2 real (stubs EGL/GLES2)
  → OK.
- El APK solo se compila en CI (NDK).

### Falta (orden sugerido)

1. Mover/redimensionar el rect creado (selector + knotholders — ShapeEditor ya
   esta stubeado esperando).
2. Más tools con la misma receta (`rect_tool_port` como plantilla).
3. Documento XML real (`svg:rect` ↔ SPRect) reemplazando el registry.
4. Undo/redo (DocumentUndo ya es stub), snapping real, capas.
5. Vulkan (ThorVG wg): fuera de scope de la alpha.

---

## Contexto anterior (resumen breve)

- Decidido: app Android nativa NDK; motor de render ThorVG (backend GL por
  ahora; Vulkan/wg post-fase). Gate de benchmark ThorVG-GL vs Rive-ORE
  suspendido por decisión (construir > investigar); informe previo en
  /sdcard/Download/thorvg-vs-rive-android-madurez-verificado.md.
- `android/` actual del repo = vía GTK4 abandonada; el prototipo vive en
  `prototype/` aparte.
- Build por CI (repo InkStudio), no local (sin NDK host).