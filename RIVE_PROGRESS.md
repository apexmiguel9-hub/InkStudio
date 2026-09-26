# RIVE_PROGRESS.md — Port de InkAlpha (alpha rect/select) a Rive low-level (Vulkan)

Experimento acotado y paralelo al port ThorVG (branch `alpha-thorvg-rect`, intacto).
Branch: `alpha-rive-experiment`. Objetivo final: tabla comparativa lado a lado
(tiempos reales, bugs de primera integración, fricciones de API, calidad por
píxeles en el g56, fluidez al tacto) + recomendación de migrar o quedarse.

## Qué se porta (BYTE-igual al ThorVG a nivel de tools)
- `sham/`, `tools/`, `registry`, dispatch de touch: engine-agnostic, sin cambios.
- Escena: fondo 0xFFEFEFF4, fill 0xF03366FF, stroke 1.5 #121420, cue #2A2D35,
  handles blancos, rubberband 0x3C3366FF/#FF2E5FFF. Solo cambia `renderer.cpp`.
- UI: GLSurfaceView → SurfaceView + SurfaceHolder.Callback (`nativeSurface`).
- Threading: hilo de render nativo dueño de tool+registry+Vulkan (single-owner);
  JNI solo encola tasks.

## Stack Rive
- `rive::gpu::RenderContext` (Vulkan impl = el Factory) + `RiveRenderer` +
  `RenderPath`/`RenderPaint`, presentando por swapchain ANativeWindow propio
  (`rive_vk_bootstrap` dentro de rive-runtime).
- Tag fijado: `runtime-v0.1.465`. NDK exacto: r27c (27.2.12479018) — **hard pin**
  en `rive_build_config.lua` (error, no warning).
- Build: `--with_vulkan` solo; ninja targets `rive rive_pls_renderer`;
  out = `out/android_arm64_release`; `rive_vk_bootstrap` compilado dentro de
  nuestro target CMake (premake solo lo compila en path_fiddle).
- SPIR-V vendored en repo (sin glslang en CI).

## Ledger de tiempos (real, por pieza)
| Pieza | Tiempo | Notas |
|---|---|---|
| Investigación API rive-runtime (clone tag, headers, tests/common/testing_window_android_vulkan.cpp, rive_vk_bootstrap) | ~45 min | contra el HEAD del tag real, no docs |
| Código: renderer_vk (Rive/Vulkan) + JNI nativeSurface + MainActivity + CMake + build script + workflow | ~60 min | sin compilar aún; 2 fixes de primera pasada (touchImpl/setToolImpl miembros, vulkan.h en header, ANDROID_STL=c++_static) |
| CI run #1 (falla 10s): NDK equivocado | ~10 min | runner con NDK 27.3/28.2/29.0; script elegía `tail -1` = 29.0; Rive exige r27c |
| CI run #2 (falla 10s): exit 2 silencioso | ~15 min | causa raíz: runner NO tiene r27c + `sdkmanager` ausente; `ls` sobre dir inexistente → exit 2 → `set -euo pipefail` mataba el script sin output |
| Fix: NDK r27c directo de dl.google.com en CI + descubrimiento blindado | ~15 min | `|| true` en substituciones; checks explícitos con mensaje + exit 1 |
| CI run #3 (falla 82s): premake5 bootstrap | ~10 min | `src/host/os_uuid.c` fatal: `uuid/uuid.h` no está (falta libuuid-dev en el runner); build_rive.sh compila premake desde fuente en Linux |
| Fix: pre-seed binario premake5 linux (v5.0.0-beta7) en el path exacto que build_rive.sh chequea + `export RIVE_PREMAKE_TAG` | ~15 min | sin apt ni clone+make recursivo por run |
| SPIR-V vendored implementado: generado en host Debian-arm64 (glslang 15.2.0 + spirv-opt + ply 3.11; SPIR-V es arch-independiente) + seed+touch en CI antes de premake para que el make de shaders sea no-op | ~25 min | evita glslang en el runner; corpus 418 files/12MB casa con el tag pinned; README reproducible |
| **CI real (rive build)** | running… | NDK r27c OK + premake seeded + clone premake-ninja + gen premake + shaders + compile arm64 rive/rive_pls_renderer |
| Verificación g56 (pendiente) | — | pixeles BPP=4 + batería draw→select→move→scale→rotate |
| **TOTAL hasta APK** | ~2.5h + CI | vs ThorVG alpha1 ~3.5h (CI ~7 min/3 runs) |

## Bugs de primera integración (CI, con causa raíz)
| # | Síntoma | Causa raíz | Fix |
|---|---|---|---|
| 1 | `Rive requires NDK 27.2.12479018; found 29.0.14206865` (10s) | runner trae NDKs nuevos; `ls\|sort -V\|tail -1` elige el más nuevo; Rive clava r27c | preferir `ndk/27.2.12479018` explícito |
| 2 | exit 2 SIN output (10s) | runner no tiene r27c; `sdkmanager` no existe en PATH del runner (paso best-effort no podía instalar); `ls -d r27c 2>/dev/null` → exit 2 → pipefail+set -e mata el script mudo | workflow: `curl` el zip oficial + unzip; script: substituciones con `\|\| true` |

## Pendiente
- [ ] Primer APK verde (rive build)
- [ ] Instalar en g56 + verificar por píxeles (rect, select/move+rubberband, scale/rotate con quad rotado)
- [ ] Métricas de frame-time/fluidez al drag
- [ ] Tabla comparativa final + recomendación