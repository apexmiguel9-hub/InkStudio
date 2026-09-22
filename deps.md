# Pins del port Inkscape → Android arm64

Auditoría de dependencias, versiones y flags. **Las libs NO viven en esta
repo**: se clonan en CI de las dos repos hermanas (2 clones = 0 duplicación
de GB, 0 cuota LFS nuestra, sin fusionar repos).

| árbol | repo | aporta |
|---|---|---|
| `.blender-libs` | `projects.blender.org/blender/lib-android_arm64` (oficial, LFS con pull dirigido) | 13 dirs tier1: zlib 1.3.1, png 1.6.58, jpeg 2.1.3, webp 1.6.0, openjpeg 2.5.3, freetype 2.13.3, harfbuzz 10.0.1 (+subset), fribidi 1.0.12, xml2 2.14.6, epoxy 1.5.10, potrace 1.16, zstd 1.5.7, brotli 1.0.9. **Blender no incluye `.pc`** → los genera `scripts/gen-pc.sh` (15 shims, incl. freetype2 26.2.20 y epoxy_has_egl=1) |
| `.libs` | `apexmiguel9-hub/lib-inkscape-android_arm64` (tier1+tier2, todo verde en CI) | GNOME stack: **GTK 4.22.5** (`libgtk-4.so` 28.5 MB, forma de entrega = .so), **gtkmm 4.14.0** (`.a` 12.8 MB con **gdkmm fusionado vía `link_whole`** — gdkmm es lib interna `install:false` en gtkmm 4.x, sin `gdkmm-4.0.pc` por diseño), glib 2.88, glibmm 2.78.1 (+giomm), cairo 1.18.6 (FT on), pango 1.58, pangomm 2.58, cairomm 1.15.4, sigc++ 3.6.0, fontconfig 2.17.1, gdk-pixbuf 2.42.12 (loaders estáticos), graphene, tiff 4.7.2, xkbcommon 1.7.0, icu, gsl, bdw-gc, lcms2, gettext (libintl), libiconv, libxslt, libffi, pcre2, pixman, boost (headers + `libboost_stacktrace_basic.a`), expat, atomic_ops, double-conversion (config CMake) |

## Requisitos REQUIRED de Inkscape (auditoría de `DefineDependsandFlags.cmake`)

pkg-config (16 módulos + pins — pre-flight en el workflow antes del cmake):

| módulo | pin Inkscape | proveedor |
|---|---|---|
| harfbuzz | >= 2.6.5 | shim (Blender 10.0.1) |
| pangocairo | >= 1.44 | .libs/pango |
| pangoft2 | — | .libs/pango |
| fontconfig | — | .libs |
| gmodule-2.0 | — | .libs/glib |
| bdw-gc | — | .libs (pc real) |
| lcms2 | — | .libs (pc real) |
| icu-uc | — | .libs/icu (pc real: icu-uc/i18n/io) |
| cairomm-1.16 | (>= 1.15.4 via gtkmm) | .libs/cairomm 1.15.4 |
| pangomm-2.48 | >= 2.50.0 | .libs/pangomm 2.58 |
| gdk-pixbuf-2.0 | >= 2.35.5 | .libs 2.42.12 |
| graphene-1.0 | — | .libs |
| gtk4 | >= 4.14.0 | .libs 4.22.5 (+ gtk4-unix-print) |
| glibmm-2.68 | >= 2.78.1 | .libs 2.78.1 (+ giomm-2.68) |
| gtkmm-4.0 | >= 4.13.3 | .libs 4.14.0 (pin exacto del fallback de Inkscape) |
| sigc++-3.0 | >= 3.6 (+ `-DSIGCXX_DISABLE_DEPRECATED`) | .libs 3.6.0 |
| epoxy | — | shim (Blender) |

CMake (todos vía `CMAKE_PREFIX_PATH` contra ambos árboles, sin `.pc`):
PNG/Potrace/ZLIB/LibXml2/LibXslt/Iconv/Intl/GSL/Boost/`double-conversion
CONFIG`. Notas: `FindPotrace` = paths puros (no necesita shim); Boost en
UNIX/Android cae en el fallback `stacktrace_basic` (nuestra `.a` ✓) porque
`stacktrace_backtrace` no existe; `ENABLE_NLS` queda OFF con
`-DWITH_NLS=OFF` (así no se añade `po/` → sin submodule de traducciones).

## Flags del primer pase

```
-DWITH_POPPLER=OFF -DENABLE_POPPLER_CAIRO=OFF   # tier3; se auto-apagarían igual
-DWITH_LIBWPG=OFF -DWITH_LIBVISIO=OFF -DWITH_LIBCDR=OFF
-DWITH_LIBSPELLING=OFF -DWITH_GSOURCEVIEW=OFF
-DWITH_IMAGE_MAGICK=OFF -DWITH_GRAPHICS_MAGICK=OFF
-DWITH_GNU_READLINE=OFF -DWITH_CAPYPDF=OFF -DWITH_NLS=OFF
-DBOOST_ROOT=.libs/boost -DBoost_NO_SYSTEM_PATHS=ON
-DCMAKE_TOOLCHAIN_FILE=scripts/android-aarch64.cmake   # NDK r30, API 31, c++_static
```

`glslc`/`mm-common`/`gstreamer` sólo se exigen si NO encontramos gtkmm/glibmm
(los encontramos → bloque saltado). `WITH_CROSSINK` es de Crossroads/MSYS2 —
nuestro camino es el toolchain CMake estándar.

## Fuentes

- **Inkscape**: `https://gitlab.com/inkscape/inkscape.git` (fuente viva;
  `master` = `2882d84`, 2026-09-21). **`github.com/inkscape/inkscape` está
  STALE desde 2022-03-03** y su tree ni siquiera trae `.gitmodules` ni
  `src/3rdparty/` → run #2 del port murió con `pathspec ... did not match any
  file(s) known to git` en los 4 submodules. `actions/checkout` sólo clona de
  GitHub → el workflow usa `git clone` directo a gitlab (soporta rama/tag
  shallow; sha vía `--filter=blob:none`).
- Submodules requeridos: `2geom`, `libcroco`, `libdepixelize`, `libuemf`,
  **`share/themes`** (`share/CMakeLists.txt:68` = `add_subdirectory(themes)`
  **incondicional** — run #4 lo probó; la auditoría previa decía "sólo
  install(DIRECTORY)" y era **falso**); `po` gateado por `NLS=OFF` (la run #4
  pasó de largo de `po` ✓), `extensions` sólo `install(DIRECTORY)` (sin paso
  install en el workflow — al empaquetar el APK sí hará falta), `capypdf`
  con `WITH_CAPYPDF=OFF` (**pero ojo**: el OFF necesita además el parche de la
  lección de abajo — sin él el generate muere pidiendo el target igualmente).

## Boost (lecciones runs #3 y #4 del port)

`find_package(Boost 1.19.0 REQUIRED)` (DefineDependsandFlags:371):

- **Run #3**: ni módulo ni config ⇒ *"By not providing FindBoost.cmake...
  asked ... a package configuration file"*. Ojo: el cmake **oficial ≥ 3.30 ya
  NO instala `Modules/FindBoost.cmake`** (el de Debian sí lo trae — por eso
  en local "existía"), y el boost de Blender trae `include/` +
  `libboost_stacktrace_basic.a` pero **sin `BoostConfig.cmake`**.
- **Run #4**: `CMP0167` (`inkscape/CMakeLists.txt:4-5` = `NEW`) — la doc
  oficial dice NEW = "search for upstream BoostConfig directamente", pero
  **sólo excluye el FindBoost built-in**: un `FindBoost.cmake` en
  `CMAKE_MODULE_PATH` (nuestro) **sigue ganando la búsqueda del modo módulo
  antes que el config** ⇒ el stack de la run #4 es
  `Modules/FindBoost.cmake:13 (include)` ← `find_package:371`, y el test
  local lo replicó. **Mi lectura inicial ("NEW ⇒ jamás carga FindBoost") era
  FALSA.** Falló el `include`: copiamos el entry a `Modules/` y NO su
  `boost-find-core.cmake` (incluido vía `CMAKE_CURRENT_LIST_DIR`) ⇒
  `include could not find` ×3. Réplica local: core al lado del entry →
  VERDE; sin él → el error exacto de la run #4.

Fix (`scripts/cmake/`, testado en **ambos** modos con las 3 llamadas de
Inkscape → ALL-GREEN): lógica única `boost-find-core.cmake` + `FindBoost.cmake`
(+ core) copiados a `CMakeScripts/Modules/` — **es lo que manda** — y
`BoostConfig.cmake`/`BoostConfigVersion.cmake` (+ core) en
`<boost>/lib/cmake/boost-<ver>/` (versión **leída del `version.hpp`**, sin
hardcode) + `-DBoost_DIR` como **red** para refs sin el módulo.
Semántica: el call 373 (`stacktrace_backtrace`, no disponible) deja
`BOOST_FOUND=FALSE` → Inkscape cae al 378 con `stacktrace_basic` = nuestra `.a`.

## NDK (lección run #4 del port)

El Configuration Summary mostró `CMAKE_C_COMPILER: /usr/local/lib/android/sdk/ndk/27.3...`
= **el NDK preinstalado del runner**, no nuestro r30 (que sí estaba en disco:
cache hit). Causa: el toolchain consulta `ENV{ANDROID_NDK_HOME}` **antes** que
el glob `$HOME/android-ndk-*`, y ubuntu-latest expone esa variable apuntando a su 27.x.
Fix triple: `Setup NDK` exporta `ANDROID_NDK_HOME=$HOME/android-ndk-<ver>` vía
`GITHUB_ENV`, el configure pasa `-DCMAKE_ANDROID_NDK` explícito (precedencia
máxima en el toolchain) y una **puerta** exige `android-ndk-<ver>/` en
`CMAKE_C_COMPILER` dentro de `configure.log`. Riesgo si no se arreglaba:
mezclar libc++/clang r27 en el port con libs construidas con r30.

## `.pc` con rutas absolutas (lección runs #5/#6 del port)

`pkg-config --exists` **NO valida rutas** ⇒ el pre-flight pasó y el fallo sólo
estalló en el *generate*: `Imported target "PkgConfig::ICU_UC" includes
non-existent path ".../lib-inkscape-android_arm64/.work/staging/icu/include"`
×2. **Raíz en la repo de libs** (`build/build.sh` harvest): normalizaba con
`s|^prefix=.*|prefix=${pcfiledir}/../..|` — **no matchea `prefix = ` con
espacios** (estilo ICU ⇒ `icu-uc/icu-io/icu-i18n.pc` rotos) y `gsl.pc` lleva
rutas absolutas **incrustadas literalmente** en `exec_prefix/libdir/
includedir/Libs/Cflags` (no derivan de `${prefix}` ⇒ aunque el prefix
matchee, sigue roto). `fontconfig.pc` sólo las tiene en `sysconfdir` etc.
(no entran en Cflags ⇒ no bloquean, pero se normalizan igual).

- **Fix port**: paso *pc overlay* reescribe cualquier
  `/home/runner/.../staging/<name>` de **todos** los `.pc` de `.libs` a
  `${pcfiledir}/../..[/sufijo]` (2 reglas sed, testado contra pkg-config real
  con la estructura `<name>/lib/pkgconfig/`) + gate `grep /home/runner` sobre
  `.libs` **y** `pc-overlay`.
- **Fix raíz** (repo libs): mismo sed de 2 reglas en `build.sh` (sustituye al
  `^prefix=` no-espaciado) para que el próximo harvest nazca limpio.
- **Defensa**: el pre-flight ahora comprueba que **todos** los `-I`/`-L` que
  resuelvan `--cflags`/`--libs` apuntan a dirs reales (19 módulos) → esta
  clase entera de bug muere ANTES de cmake.

## Generator expression con variable literal (lección runs #5/#6 del port)

`src/CMakeLists.txt:339`:

```cmake
$<$<BOOL:WITH_CAPYPDF>:Inkscape::CapyPDF>   # BUG upstream
```

Las genexps **no expanden variables**: `WITH_CAPYPDF` es un *string literal* y
`$<BOOL:<string-no-falso>> = 1` ⇒ el link al target se exige **siempre**, pero
el target sólo se crea dentro de `if(WITH_CAPYPDF)`
(`DefineDependsandFlags.cmake:195`) ⇒ con `-DWITH_CAPYPDF=OFF` el generate
muere con *"Target … links to Inkscape::CapyPDF but the target was not found"*.
Barido completo del árbol: es el **único** `$<BOOL:VAR>` literal (el resto de
opciones OFF no lo sufren). Fix: parche al clone efímero
`BOOL:WITH_CAPYPDF>` → `BOOL:${WITH_CAPYPDF}>` (`${...}` SÍ se expande al
parsear ⇒ `$<BOOL:OFF>` = 0) + 2 gates. Alternativa descartada por más pesada:
submodule `capypdf` + `WITH_CAPYPDF=ON`.

## Historial de runs (port)

- **#1** `35719592283`: puerta LFS `head -c 5` (`!<arc` ≠ `!<arch>`) → 6 bytes exactos.
- **#2** `35719922554`: `actions/checkout` contra el mirror GitHub STALE
  (2022, sin `.gitmodules`) → `git clone` a gitlab.
- **#3** `35734275593`: primer contacto con el configure; muere en
  `find_package(Boost)` (sin FindBoost built-in ni BoostConfig).
- **#4** `35737210796`: **llega AL configure** (submodules gitlab, gen-pc,
  pre-flight 16+6 ✓) y muere con 3 errores: (a) FindBoost entry sin su core
  en `Modules/`, (b) `share/themes` sin inicializar
  (`add_subdirectory` incondicional), (c) NDK 27.3 del runner pisando el r30
  vía `ANDROID_NDK_HOME`.
- **#5** `35741814037`: las 3 fixes de #4 **verificadas** (Boost ✓,
  5 submodules ✓, NDK r30 ✓, todos los find/link del configure ✓ — gtk4
  4.22.5, gtkmm 4.14.0, pangomm 2.58.0…) ⇒ llega a *Generate* y muere con 2
  clases nuevas: (a) `.pc` con `prefix` absoluto del CI de las libs
  (ICU_UC ×2), (b) genexp `$<BOOL:WITH_CAPYPDF>` literal (target no creado
  con OFF).
- **#6** `35742501729`: re-run de **confirmación de log** (despachada SIN los
  fixes, para capturar el detalle exacto de ambos errores) ⇒ mismos 3 mensajes,
  detalle leído y fixes aplicados en esta tanda.

## Historial de runs (libs hermanas)

tier1 = run 8 ✓ · tier2: …→ 19 (cairo FT + gtk4 ✓) → 20 (sigc++ ✓/glibmm
char_traits) → 21 (glibmm+cairomm+pangomm ✓/gtkmm typedef) → 22 (wrap_init ✓
/iconpaintable single-include) → **23 = TIER2 COMPLETA ✓** (gtkmm 353/353).
