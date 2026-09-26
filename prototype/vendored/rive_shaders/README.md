# Vendored rive PLS shaders (SPIR-V corpus)

The PLS (pixel local storage) Vulkan backend of rive-runtime needs its shaders
compiled to SPIR-V at build time. The premake step runs:

    make -C <rive>/renderer/src/shaders OUT=<out>/include/generated/shaders \
         FLAGS="-p <ply>" spirv

which requires `glslangValidator` + `spirv-opt` on the PATH. Those are NOT on
the GitHub runner, so the generated corpus is vendored here and seeded into
`out/android_arm64_release/include/generated/shaders` by
`prototype/scripts/build_prototype_rive.sh` right after cloning rive-runtime
(**before** premake runs) and `touch`-ed so the mtime-based Makefile sees every
target up to date and skips the toolchain entirely (no-op).

Why it's safe to vendor: SPIR-V bytecode is architecture-independent, and the
`runtime-v0.1.465` tag is pinned (see commit / RIVE_PROGRESS.md). The corpus
only needs regenerating if the rive tag moves.

## How this corpus was generated (reproducible)

Host: Debian 13 (trixie), glslang-tools 15.1.0 (glslangValidator 15.2.0),
spirv-tools 2025.1 (spirv-opt), python3-ply 3.11, make 4.4.1.

    git clone --depth 1 --branch runtime-v0.1.465 https://github.com/rive-app/rive-runtime
    git clone --depth 1 --branch 3.11 https://github.com/dabeaz/ply.git
    cd rive-runtime/renderer/src/shaders
    make -j$(nproc) OUT=/tmp/shader-out FLAGS="-p /path/to/ply" spirv

Then the full contents of `/tmp/shader-out/` were copied here.

## Contents (418 files, 12 MB)

- `spirv/*.spv` + embedding `spirv/*.h` (compiled all shader variants)
- `*.hpp` — minified GLSL as C++ raw-string literals
- `*.minified.glsl`, `*.exports.h` — minify.py outputs
- `glsl.stamp` — minify batch stamp

Regenerate = rerun the command above against the same tag with a modern
glslang; do NOT edit generated files by hand.