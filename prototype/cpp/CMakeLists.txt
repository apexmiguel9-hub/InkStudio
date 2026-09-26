# InkAlpha native module — Rive low-level Vulkan canvas + ported Inkscape tools
# (RIVE EXPERIMENT branch).
cmake_minimum_required(VERSION 3.22)
project(InkAlphaNative LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# RIVE_ROOT = rive-runtime checkout, RIVE_BUILD = its premake out dir
# (out/android_arm64_release) containing librive.a + librive_pls_renderer.a.
if(NOT DEFINED RIVE_ROOT OR NOT DEFINED RIVE_BUILD)
    message(FATAL_ERROR
        "Pass -DRIVE_ROOT=<rive-runtime> and -DRIVE_BUILD=<premake out dir>")
endif()

set(RIVE_INC        ${RIVE_ROOT}/include)
set(RIVE_RENDERER_INC ${RIVE_ROOT}/renderer/include)
set(RIVE_VKB_INC    ${RIVE_ROOT}/renderer/rive_vk_bootstrap/include)
set(RIVE_BOOTSTRAP_DEPS ${RIVE_ROOT}/build/dependencies)
set(VK_HEADERS_INC  ${RIVE_BOOTSTRAP_DEPS}/vulkan_headers/include)
set(VMA_INC         ${RIVE_BOOTSTRAP_DEPS}/vulkan_memory_allocator/include)
set(RIVE_GEN_INC    ${RIVE_BUILD}/include)

file(GLOB RIVE_LIBS
     ${RIVE_BUILD}/rive/librive.a
     ${RIVE_BUILD}/renderer/librive_pls_renderer.a)
if(NOT RIVE_LIBS)
    message(FATAL_ERROR "No librive*.a under ${RIVE_BUILD} — run build_rive.sh first")
endif()

# rive_vk_bootstrap is NOT part of librive_pls_renderer.a — the premake only
# compiles it into the path_fiddle example (renderer/premake5.lua). Compile it
# into our own target instead (same sources/bootstrap_project.lua picks).
file(GLOB RIVE_VKB_SRC ${RIVE_ROOT}/renderer/rive_vk_bootstrap/src/*.cpp)

add_library(inkalpha SHARED
    jni_bridge.cpp
    renderer.cpp
    tool/rect_tool_port.cpp
    tool/select_tool_port.cpp
    sham/tool_base.cpp
    sham/snap.cpp
    sham/desktop.cpp
    sham/seltrans.cpp
    ${RIVE_VKB_SRC}
)

target_include_directories(inkalpha PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/sham
    ${CMAKE_CURRENT_SOURCE_DIR}/tool
    ${RIVE_INC}
    ${RIVE_RENDERER_INC}
    ${RIVE_VKB_INC}
    ${RIVE_GEN_INC}
    ${VK_HEADERS_INC}
    ${VMA_INC}
)

# Same defines the rive premake sets when building with --with_vulkan
# (renderer/premake5_pls_renderer.lua): load all Vulkan entry points
# dynamically via the instance proc-addr (VK_NO_PROTOTYPES).
target_compile_definitions(inkalpha PRIVATE
    RIVE_VULKAN
    VK_NO_PROTOTYPES
    VMA_STATIC_VULKAN_FUNCTIONS=0
    VMA_DYNAMIC_VULKAN_FUNCTIONS=1
)

target_compile_options(inkalpha PRIVATE -Wall -Wextra)

# rive_pls_renderer (GPU) before rive (core); vulkan = libvulkan.so at runtime.
target_link_libraries(inkalpha PRIVATE
    ${RIVE_BUILD}/renderer/librive_pls_renderer.a
    ${RIVE_BUILD}/rive/librive.a
    vulkan
    android
    log
)