# Inkscape-android_arm64

Port de **Inkscape** a **Android arm64**, manteniendo **GTK4 + gtkmm4** (el
objetivo NO es reemplazar el toolkit). Es la continuación directa del port de
Blender del mismo usuario: mismos patrones de CI, mismo NDK, mismas libs
oficiales.

## Estructura

| archivo | función |
|---|---|
| `scripts/android-aarch64.cmake` | Toolchain CMake: NDK r30, API 31 (`aarch64-linux-android31`), `c++_static`, `FIND_ROOT_PATH_* = BOTH` para que las libs arm64 pre-compiladas de rutas host se encuentren vía `CMAKE_PREFIX_PATH` |
| `.github/workflows/build-inkscape.yml` | 3 clones en CI (Blender oficial con LFS dirigido · nuestras libs · Inkscape + 4 submodules), pre-flight de los 16 módulos pkg-config REQUIRED + 6 version-pins, configure con tier3 OFF, build con `-k0` y diagnósticos |
| `deps.md` | Pins verificados: quién provee cada requisito REQUIRED de Inkscape, flags del primer pase, historial de runs |

## Las dos repos de dependencias

1. **`projects.blender.org/blender/lib-android_arm64`** — binarios oficiales
   de Blender (zlib, png, freetype, harfbuzz, xml2, epoxy, potrace, brotli…).
   En CI se clona con `GIT_LFS_SKIP_SMUDGE=1` y se hace `git lfs pull` sólo
   de los 13 dirs que usa Inkscape (puerta: cabecera `!<arch>` real).
2. **[`lib-inkscape-android_arm64`](https://github.com/apexmiguel9-hub/lib-inkscape-android_arm64)**
   — tier1 (14) + tier2 GNOME (14) construidas por GitHub Actions:
   **GTK 4.22.5** (`.so`), **gtkmm 4.14.0** (`.a`, con gdkmm dentro),
   glib/glibmm/pango/cairomm/pangomm/sigc++/cairo/fontconfig/gdk-pixbuf…
   Sus `.pc` son reales; las de Blender llevan los 15 shims de `gen-pc.sh`.

## Cómo lanzarlo

```bash
gh workflow run build-inkscape \
  --repo apexmiguel9-hub/Inkscape-android_arm64 --field ref=master
```

Inputs: `ref` (rama/tag/sha de inkscape) y `ndk` (`r30`; si da problemas
probar `r30-beta1`, la que usa Blender).

## Plan de trabajo

1. ✅ Dependencias completas en CI (tier1 + tier2 de la repo hermana).
2. ▶ **Primer pase de compilación de Inkscape (esta repo)** — se esperan
   errores (misma iteración probada: dispatch → diagnóstico → fix → dispatch).
3. Gestos (futuro: `src/ui/controllers/` + `src/display/event/`).
4. Menú de atajos (futuro: `src/ui/shortcuts.cpp` + `src/actions/`).

## Decisiones fijadas

- **gtkmm 4.14.0 exacto** = el pin del `ExternalProject` de fallback de
  Inkscape (`DefineDependsandFlags.cmake`); con gtk 4.22.5 requirió un parche
  quirúrgico de skew (`GtkIconPaintable` derivable→final) documentado en la
  repo de libs.
- **`libgtk-4.so` como `.so`** y el resto estático: así está cosechado el
  tier2; los riesgos de duplicación de símbolos gtk4↔cairo/pango/glib son el
  siguiente eslabón a vigilar al enlazar.
- Tier3 (poppler, libwpg/visio/cdr, gtksourceview-5, libspelling, ImageMagick,
  jemalloc, readline) queda OFF en el primer pase — importaciones concretas
  se activarán después.
