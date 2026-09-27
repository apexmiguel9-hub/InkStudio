# INKSCAPE_PROGRESS.md

Progreso del port de Inkscape a Android nativo (sin GTK4/Cairo). Build vía CI
(GitHub Actions, repo `apexmiguel9-hub/InkStudio`); fuentes de Inkscape se
usan como referencia/port local (`/root/inkscape`).

Repos: código del prototipo en `prototype/` (rama `alpha-thorvg-rect`).
Estado general: **Alpha 2 — SelectTool funcional (mover + escala + rotar)**.

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

## ALPHA 2 — SelectTool: mover + escala + rotar (port + 3 bugs) — 2026-09-26

**Objetivo cumplido:** port del SelectTool de Inkscape (selección por tap,
handles de escala/rotación, body-drag) y los 3 bugs reportados por el usuario
en el g56, todos verificados por píxeles en el dispositivo.

### Tiempo real del proceso

| Etapa | Tiempo real (sesión) | Notas |
|---|---|---|
| Port select_tool.cpp + shams (seltrans, desktop hit-test, sp_rect.xform) | ~90 min | copia literal del algoritmo; shims nuevos = donde viven los bugs reales |
| Repro + análisis de píxeles en dispositivo (decoder propio con `*bpp`, BPP=4) | ~60 min | el análisis previo con `*3` era artefacto (zebra, "cue desplazado") |
| Root cause bugs #2/#3: convención de `tvg::Matrix` (fila vs columna) leída del tag v1.1.2 | ~25 min | tvgMath.h `operator*=(Point&, Matrix&)`: `x'=e11·x+e12·y+e13` |
| Fix toTvg (traslación a e13/e23) + fix draw(true) (clear de FBO entre frames) | ~15 min | 2 commits, 2 ciclos CI |
| Batería de verificación en dispositivo (draw→select→move→scale→rotate) | ~45 min | aserciones relativas + handles pegados + anclaje |
| Desactivar/reactivar rotación (escala primero, rotar después — decisión usuario) | ~10 min | motor de rotación intacto mientras tanto |
| **Total** | **~4 h** | la lección más cara: convención de matrices de ThorVG |

### Root causes (verificadas contra el tag v1.1.2, no adivinadas)

1. **Lienzo negro al primer arranque** (bug #1): la tool no existía hasta el
   primer touch y el primer frame se dibujaba vacío. Fix: `ensureTool()` dentro
   de `frame()` (renderer.cpp). Verificado: primer frame sin tocar = gris.
2. **Overlay/handles despegados ("al mover solo se mueve el box", "al
   redimensionar no está centrado", rotación "como una hoja")** (bugs #2/#3):
   nuestro `toTvg()` mapeaba la traslación a `e31/e32` (tercera FILA), pero
   ThorVG v1.1.2 multiplica puntos como vectores FILA (`tvgMath.h`:
   `x'=e11·x+e12·y+e13`, `y'=e21·x+e22·y+e23`): la traslación vive en
   `e13/e23` (tercera COLUMNA). Resultado: el mimo/scale/rotate correctos en
   el modelo (el overlay/docBBox los seguía) pero el FILL renderizado en el
   origen — caja despegada, resize des-anclado, rotación girando alrededor
   del origen en vez del centro. Fix (renderer.cpp `toTvg()`):
   `a→e11, c→e12, e→e13 / b→e21, d→e22, f→e23`. Esto explica los 3 síntomas
   con UNA línea.
3. **Fantasmas/stale al mover** (complemento del #2): `Canvas::draw(false)`
   no limpia el target entre frames (thorvg.h v1.1.2: `clear=true` clears the
   target buffer before drawing); el blit solo sobrescribe la región
   actualizada → quedaba la silueta del frame anterior. Fix: `draw(true)`
   (nuestra escena es opaca — el rect de fondo cubre todo — así que el clear
   es invisible cuando no hay cambios).

### Evidencia por píxeles (batería en el g56, decoder `*bpp` BPP=4)

- S2 select: 8 handles blancos exactos al bbox `(401,402)-(883,688)` (n=64
  cada uno) + cue pegado (371 muestras del borde).
- S3 mover: fill `(401,402)→(581,512)`, Δ=(180,110)≈gesto; los 8 handles
  PEGADOS al nuevo bbox, sin fantasma del rect viejo.
- S4 escalar: arrastrar TL → TL sigue al dedo y BR anclado (el fix #2 es el
  anclaje); handles pegados.
- Rotación (reactivada tras validar escala): gira alrededor del centro del
  bbox (acumulación incremental `last_ang`/`rot_accum`, sin flip).

### Alpha 2b — el box se acopla al rotar (overlay por quad rotado) + máquina de estados

**Reporte del usuario:** "funciona el rotate, pero el box se hace un pixel mas
grande o algo asi... el box es mas visual que problema funcional". El overlay
(cue + handles) se dibujaba desde `docBBox()` = **AABB** del rect rotado, que
siempre es más grande que el rect girado → el "box" no se acoplaba al fill.

**Root cause + fix (verificado contra seltrans.cpp real de Inkscape):**
Inkscape dibuja su caja de selección durante el drag como las esquinas del
bbox **transformadas** por el affine (`_l[i]->set_coords(_bbox->corner(i) *
affine)`, `src/seltrans.cpp:407`) — la caja **rota con el contenido**. El
port la dibujaba como AABB. Fix en 3 piezas:

1. `selectionQuad()` (seltrans): el contorno transformado de la selección —
   item único = sus 4 `docCorners()` (quad bajo el xform actual); multi-
   selección = unión AABB (inalterado).
2. `handlePositions()`: los 8 handles de escala sobre el quad rotado
   (esquinas + puntos medios de arista) y los 4 de rotación sobre las
   esquinas del quad + 28px en su diagonal (antes: AABB). El crosshair del
   pivot sigue en el centro.
3. `moveHandle()` scale: las anclas/denominadores usan el `quad0` congelado
   en `grab()` (marco rotado) — para un rect sin rotar el quad == AABB, la
   math es idéntica (sin regresión en la batería de escala ya validada).
4. `renderer.cpp` cue: polilínea cerrada sobre `docCorners()` en vez de
   `appendRect` del AABB.

**Evidencia por píxeles (g56, APK `019b3e6`, decoder `*BPP` BPP=4):** tras
rotar el rect (drag del handle BR), el fill es un paralelogramo perfecto
(aristas opuestas paralelas: (392,51) y (-32,243)), con centro invariante
`(598.3,523.8) ≈ (598.5,524.0)` — el pivot no se mueve, no hay "hoja". El cue
oscuro queda a **1.0px de las 4 esquinas rotadas** (antes: slack diagonal
del AABB de decenas de px) y los 4 handles de rotación cabalgan las esquinas
del quad + 28px en el marco rotado. En escala (sin rotar) la math sobre el
quad == AABB, por lo que la batería ya validada de move/scale quedó intacta.

**Bonus (máquina de estados, explica captures "raras" del harness):** el rect
se **auto-selecciona al crearlo** (fiel a Inkscape); el 1er tap sobre un item
ya seleccionado = `increaseState()` → entra en ROTACIÓN (no escala). Esto es
comportamiento de Inkscape (2º click conmuta scale↔rotate), no un bug — pero
fue lo que hizo que las capturas de battery3 mostraran 4 handles de rotación
"sin que nadie rotara". El tap que no conmuta es porque aterrizó en el
crosshair del pivot (grab del centro a <40px).

- **Convención de matrices del motor = el bug más caro**: leer `tvgMath.cpp`
  del tag ANTES de escribir el mapeo doc→tvg. Un mapeo "obviamente correcto"
  (traslación abajo a la derecha) estaba muerto en silencio para ThorVG.
- El decoder de PNG del sandbox SIEMPRE con `(y*W+x)*BPP`; cualquier análisis
  con `*3` produce artefactos (zebra, overlays "desplazados").
- `Canvas::draw(false)` acumula el buffer GL: en escenas opacas el omitir el
  clear ahorra tiempo solo hasta que algo se mueve; el ghost es el síntoma.
- Auto-layout: toda la math de tools/renderer vive en espacio-doc; `w2d/d2w`
  y `toTvg()` son el pivote único para mapear a pantalla (seam comentado en
  `desktop.h`); nada de offsets de pantalla hardcodeados en las tools.

### Implementación (completa en C++)

| Archivo | Qué añade |
|---------|-----------|
| `sham/sp_rect.h/.cpp` | `nodeAt`, `handleAt`, `moveNode`, `moveHandle` (Option 1: shim sobre rect) |
| `tool/node_tool_port.h/.cpp` | `NodeTool` hereda `ToolBase`; `root_handler`: grab node/handle, drag→move, release→ungrab; se refresca `rect` desde selección en cada press |
| `renderer.cpp` | `TOOL_NODE=2`, `drawNodeOverlay(SPRect*)`: 8 cuadros 10×10 (stroke #101014) en `docCorners()` + mid-aristas + 4 círculos rotación (r=7, blanco) en esquinas+28px diagonal + crosshair centro |
| `MainActivity.java` | `TOOL_NODE=2`, botón "◆ Nodos" en toolbox |
| `CMakeLists.txt` | `node_tool_port.cpp` + `sp_rect.cpp` |

### Estado actual (2026-09-27)

**C++ implementation: COMPLETA** — NodeTool Option 1 compilada y vinculada (CI verde en run 36240268838).

**Bloqueado por bugs de integración Java/C++:**

| Bug | Síntoma | Causa probable |
|-----|---------|----------------|
| **Toolbar initial selection** | App lanza con RectTool seleccionado (botón 2), no SelectTool (botón 1). `toolbox.select(TOOL_SELECT)` no resalta botón 1. | Java `Toolbox.select()` usa `i == current` pero `current = id` (tool ID). Índices y tool IDs coinciden (0,1,2), pero visualmente botón 1 (Rect) queda seleccionado. |
| **SelectTool no selecciona** | Tap en centro de rect → nada ocurre. Sin overlay (cue/handles). | Posibles causas: (a) coordenadas touch no mapean a doc-space del rect, (b) `handleClick` no se ejecuta, (c) `selection.set()` no notifica a `SelTrans`, (d) `SelTrans.visible()` devuelve false. |

**Tiempo real invertido hoy: ~5 h** (implementación C++ NodeTool: ~2.5h; debugging toolbar/selection: ~2.5h).

**Próximos pasos para desbloquear:**
1. Fixear `Toolbox.select()` en Java (añadir logs o corregir lógica de `current` vs índice).
2. Verificar que `SelectTool.handleClick` se llama y `selection.set()` actualiza `SelTrans`.
3. Si `SelTrans` lee directo de `Selection`, checkear `SelTrans.visible()` y `isEmpty()`.
4. Solo entonces testear `NodeTool` (requiere rect seleccionado).

### Falta (orden sugerido)

1. Fixear toolbar + selection (bloqueante actual).
2. Testear NodeTool Option 1 end-to-end (grab node/handle, drag, release).
3. Documento XML real (`svg:rect` ↔ SPRect) reemplazando el registry.
4. Undo/redo (DocumentUndo stub), snapping real, capas.
5. Más tools con la misma receta.
6. Vulkan (ThorVG wg): fuera de scope de la alpha.

---

## Contexto anterior (resumen breve)

- Decidido: app Android nativa NDK; motor de render ThorVG (backend GL por
  ahora; Vulkan/wg post-fase). Gate de benchmark ThorVG-GL vs Rive-ORE
  suspendido por decisión (construir > investigar); informe previo en
  /sdcard/Download/thorvg-vs-rive-android-madurez-verificado.md.
- `android/` actual del repo = vía GTK4 abandonada; el prototipo vive en
  `prototype/` aparte.
- Build por CI (repo InkStudio), no local (sin NDK host).

---

## AUDITORÍA SKIA (migración de motor canvas: ¿ThorVG → Skia?) — 2026-09-27

**Rama**: `alpha-skia-experiment`. **Motor**: Skia m156, commit
`30ff12f0e3031c536b9b1e4dd073ffe7979f9ecc` (2026-09-27). **Workflow**:
`.github/workflows/skia-audit.yml` (NDK standalone, gn/ninja, JDK 17).

### Método (evidencia real, nada simulado)

Cada hallazgo proviene de un run real de CI con URL, un APK real instalado por
adb, o logs literales del dispositivo. Regla del usuario: si Skia pesa
demasiado, el log real de fallo ES el hallazgo; se para ahí.

### Cronología de runs (auditoría de build, runs 1-7)

1. Sync `git-sync-deps` shallow → race/fallos intermitentes → **`--deep`**.
2. Build verde (run 4): `libskia.a` 68 MB, statics total 84.9 MB, árbol 13 GB;
   checkout 5s / deps ~374s / ninja ~328s.
3. Stub link runs 5-7: `SKIA_VERSION_STRING` → `SkGraphics::Init()`. Stub
   resultó engañoso (17.02 MB sin strip / 1.73 MB con strip) — no representa
   una app real.

### Cronología de runs (fase app, runs 8-15) — logs literales

| Run | Resultado | Hallazgo real (verbatim) |
|-----|-----------|--------------------------|
| 8 (36348247701) | FAIL | `GrBackendRenderTarget` incompleto (faltaba `gpu/ganesh/GrBackendSurface.h`) + **`no member named 'addRect' in 'SkPath'`** — m156 SkPath es data-oriented: solo fábricas estáticas (`SkPath::Rect/Polygon/Line/Raw`); no hay `moveTo/lineTo/addRect`. Fix: `SkPath::Rect(rect)`. |
| 9 (36348806732) | FAIL | `incomplete type 'SkColorSpace'` → `#include core/SkColorSpace.h`. |
| 10 (36349614561) | FAIL | El .so enlazó (29 MB sin strip / 5.4 MB con strip — números reales); `javac MainActivity.java:62 cannot find symbol` (lambda param `View` sin `requestRender`) → `glView.requestRender()`. |
| 11 (36350443524) | SUCCESS 12m53s | APK 2.4 MB; **crash en dispositivo: `UnsatisfiedLinkError: library "libc++_shared.so" not found`** (DT_NEEDED confirmado; `-lc++_static` no neutraliza el default del NDK) → `-static-libstdc++` + gate DT_NEEDED. |
| 12 (36351385538) | SUCCESS 10m8s | **crash: `cannot locate symbol "__real_realpath"`** — `nm`: `liballocator_shim.a` (allocator_shim_android.o) define `__wrap_*`/ref `__real_*`; el glob `lib*.a` lo coló → **allowlist canónica** (skia, skcms, png, jpeg, webp, wuffs, zlib, freetype2, expat, piex, dng_sdk, cpu-features) + gate dlopen. |
| 13 (36352186325) | SUCCESS 13m4s | **crash: `_ZN15partition_alloc8internal21PartitionAddressSpace6setup_E` + `RawPtrBackupRefImpl<false>::AcquireInternal/ReleaseInternal`** — 3 únicos huecos no-plataforma; `setup_` no existe en NINGUNA estática; libskia.a (SkSLParser.o) lo referencia; RawPtrBackupRefImpl débil en libraw_ptr.a (excluida). |
| 14 (36353162403) | FAIL 6m36s | El arg nuevo no llegó — **un apóstrofe ("skia's") en mi comentario cerró el string `--args='`** → 4 positional extra → `Need exactly one build directory to generate`. Fix: quitar apóstrofe. |
| 15 (36353580455) | **SUCCESS 9m53s** | Build completo + APK con **`skia_use_partition_alloc=false`** (fuente real: `gn/skia.gni:132` lo activa por defecto en standalone+clang; inyecta raw_ptr real de partition_alloc a todos los targets). **Reales: .so stripped = 5,579,464 B ≈ 5.32 MB; DT_NEEDED solo libs plataforma (EGL/GLESv2/android/log/m/dl/c); 0 símbolos sin resolver no-plataforma (gate verde); APK 2.4 MB.** |

### En dispositivo g56 (verificación visual/pixel del experimento mínimo)

- APK run 15 instalado por adb y **corriendo**: `skiaexp: nativeInit OK
  2400x1080` (la actividad arrancó en landscape; aceptado para el experimento).
- Swipe (300,300)→(900,700) → **el rect navy se dibuja y se queda**; el usuario
  confirmó visualmente el cuadro en pantalla. Hito del experimento mínimo
  (canvas Ganesh GL + tap→draw) verificado por ojos del usuario.

### Hallazgos m156 relevantes para el producto (NodeTool shim sobre rect)

1. **`SkPath` data-oriented** (m156): sin mutadores clásicos `moveTo/lineTo/
   addRect` — solo fábricas (`SkPath::Rect`, `Polygon`, `Line`, `Raw`).
   Impacta cualquier port de código Inkscape que construya paths incremental.
2. **`skia_use_partition_alloc` default TRUE** en standalone+clang (skia.gni:132)
   → rompe el dlopen de un .so dinámico con 3 símbolos huérfanos
   (`PartitionAddressSpace::setup_`, `RawPtrBackupRefImpl::Acquire/Release`).
   Fix probado: `skia_use_partition_alloc=false` (modo noop de partition_alloc;
   BUILD.gn:127 confirma el cambio de target).
3. **allocator shim**: no linkear `lib*.a` con glob — cola
   `liballocator_shim.a` (define `__wrap_*`, referencia `__real_realpath`).
   Usar siempre la allowlist canónica + gate `llvm-nm -D` de no-plataforma.
4. **`-lc++_static` no neutraliza** el `libc++_shared.so` por defecto del NDK:
   usar `-static-libstdc++`.

### Comparativa de tamaños (reales, run 15 vs app ThorVG)

| | Skia app (run 15) | ThorVG prototype |
|---|---|---|
| .so sin strip | 29 MB | libinkalpha.so 7.15 MB |
| .so con strip (shipped) | **5.32 MB** | (prototipo no strip) |
| APK | 2.4 MB | 2.24 MB |

### Estado tras run 15 (2026-09-27)

- **Experimento mínimo Skia VERIFICADO en dispositivo** (rect con tap+drag,
  usuario confirmó). Puente JNI→Ganesh GL estable, dlopen limpio, DT_NEEDED
  mínimo. `alpha-skia-experiment` empujado: a385f12, 9b30305, dd7b86a, 3b2ed01,
  9c2699c, c21b99d, 07e87b7, 84644ec.

### Port de las 3 herramientas → comparativa justa (runs 16-18, 2026-09-27)

- **Run 16 (36354888579, d2f32e8) FAIL** — el .so enlazó perfecto (28.23 MB
  sin strip / 5.34 MB con strip, símbolos JNI presentes, DT_NEEDED limpio);
  `javac` falló: `MainActivity.java:130 non-static method nativeSetTool(int)
  cannot be referenced from a static context` + `glView` idem (la clase
  `Toolbox` es estática) → natives `static` + `GLSurfaceView` pasado por ctor.
- **Run 17 (36355876396, a5dc251b) SUCCESS ~11 min** — **3 herramientas
  funcionando en el g56**: Selector (selección + mover + redimensionar + 4
  nodos rotación al tap siguiente + rubberband), Rectángulo (rects
  persistentes, cada drag crea uno nuevo), Nodos (overlay). Toolbar portado
  con fix de selección por tag de tool id (el bug "botón equivocado en
  launch" del prototipo ThorVG NO se reproduce aquí: Selector selected=true).
  Portrait forzado; `nativeInit OK 1080x2037`.
- **Feedback del usuario (prueba en dispositivo)**: "funciona casi bien...
  rotar sirve, crear el rectángulo sirve, deslizar el rectángulo sirve".
  2 problemas: (1) al **tocar** un nodo sin deslizar, el rect se redimensionaba
  solo hasta el dedo (falta de offset de agarre + tolerancia); (2) **nodos muy
  pequeños** (10px). La NodeTool "no hace nada" igual que en ThorVG — el
  usuario lo marca NO como problema actual.
- **Run 18 (36357151754, cea5b008) SUCCESS ~12 min — fixes aplicados y
  VERIFICADOS por el usuario en el g56 ("funciona ya lo probe xd")**:
  1. Resize SOLO con desliz: offset de agarre (el nodo queda anclado bajo el
     dedo en el grip) + tolerancia 8px — tocar sin deslizar no cambia nada.
  2. Nodos más grandes: cuadrados 22px (antes 10px), anillos r=12 (antes 7),
     hit 26px.
  3. Mid-edge handles correctos: el borde agarrado sigue al dedo, el opuesto
     queda fijo (antes el borde derecho quedaba fijo mal mapeado).
  4. Gate de tolerancia también en rotate (tap con jitter no rota).
  Logcat real de la sesión del usuario: `rect created 221x198+627x834 n=1`,
  `rect created 200x254+298x298 n=2`, `tool=2 sel=1` (Nodos), `tool=0 sel=1`.

### Conclusión de la auditoría Skia (para la decisión de motor)

- **Skia m156 corre una app GL completa con las 3 herramientas en el g56**,
  con las mismas interacciones que el prototipo ThorVG, a coste de build/
  tamaño: .so strip 5.34 MB vs ThorVG 7.15 MB sin strip (el APK Skia ~2.5 MB
  vs 2.24 MB). El dlopen/DT_NEEDED mínimo se logra con la allowlist canónica,
  `-static-libstdc++` y `skia_use_partition_alloc=false`.
- Sin embargo cada build Skia cuesta ~10-13 min de CI (deps sync ~220-374s +
  ninja ~330s) frente al build ThorVG directo, y la API m156 es data-oriented
  (SkPath sin moveTo/lineTo) — distinto mental model para portar código
  Inkscape. **Decisión de motor: SIN tomar; blockeada por aprobación del
  usuario (no cambiar el motor sin visto bueno).** ThorVG sigue siendo el
  motor de canvas final por defecto; Skia queda como experimento verificado
  para la comparativa justa.