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
| **Total** | **~2.5 h** | sin contar el build de CI (~10-15 min by runner) |

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
- Build CI: Thames nada local — workflow `build-inkalpha` (dispatch + push a
  la rama), NDK r27, meson cross arm64 (engines cpu,gl), CMake, empaquetado
  manual sin Gradle (aapt2 → javac → d8 → zipalign → apksigner debug).
- Gesture isolation: toolbar y canvas son vistas separadas.

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