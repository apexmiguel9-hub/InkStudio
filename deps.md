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
- Submodules requeridos: `2geom`, `libcroco`, `libdepixelize`, `libuemf`
  (todos gitlab); `po`/`extensions`/`themes`/`capypdf` NO en el primer pase.

## Boost (lección run #3 del port)

`find_package(Boost 1.19.0 REQUIRED)` (DefineDependsandFlags:371) falló con
*"By not providing FindBoost.cmake... asked ... a package configuration file"*.
Causa **doble** verificada:

1. `inkscape/CMakeLists.txt:4-5` fuerza `cmake_policy(SET CMP0167 NEW)` → en
   CMake ≥ 3.30 el `find_package(Boost)` va **directo a config mode** y jamás
   carga un `FindBoost.cmake` (por eso no hubo warning de "policy not set").
2. El boost de Blender (1.87.0) trae `include/` + `libboost_stacktrace_basic.a`
   pero **sin `BoostConfig.cmake`**.

Fix (`scripts/cmake/`, testado localmente en **ambos** modos con las 3
llamadas exactas de Inkscape → ALL-GREEN): lógica única en
`boost-find-core.cmake` + `BoostConfig.cmake`/`BoostConfigVersion.cmake`
(config, primario — el workflow lo instala en `<boost>/lib/cmake/boost-<ver>/`
con versión **leída del `version.hpp`**, sin hardcode, + `-DBoost_DIR`) +
`FindBoost.cmake` de respaldo (modo módulo, para refs sin la política).
Semántica: el call 373 (`stacktrace_backtrace`, no disponible) deja
`BOOST_FOUND=FALSE` → Inkscape cae al 378 con `stacktrace_basic` = nuestra `.a`.

## Historial de runs (libs hermanas)

tier1 = run 8 ✓ · tier2: …→ 19 (cairo FT + gtk4 ✓) → 20 (sigc++ ✓/glibmm
char_traits) → 21 (glibmm+cairomm+pangomm ✓/gtkmm typedef) → 22 (wrap_init ✓
/iconpaintable single-include) → **23 = TIER2 COMPLETA ✓** (gtkmm 353/353).
