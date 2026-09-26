# InkAlpha native module — Rive low-level Vulkan canvas + ported Inkscape tools
# (RIVE EXPERIMENT branch).
cmake_minimum_required(VERSION 3.22)
project(InkAlphaNative LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# RIVE_ROOT = rive-runtime checkout, RIVE_BUILD = premake out dir
# (renderer/out/android_arm64_release) with librive.a + librive_pls_renderer.a.
if(NOT DEFINED RIVE_ROOT OR NOT DEFINED RIVE_BUILD)
    message(FATAL_ERROR
        "Pass -DRIVE_ROOT=<rive-runtime> and -DRIVE_BUILD=<premake out dir>")
endif()

set(RIVE_INC         ${RIVE_ROOT}/include)
set(RIVE_RENDERER_INC ${RIVE_ROOT}/renderer/include)
set(RIVE_VKB_INC     ${RIVE_ROOT}/renderer/rive_vk_bootstrap/include)
set(RIVE_GEN_INC     ${RIVE_BUILD}/include)

# dependency.github() (build/dependency.lua) clones into <premake cwd>/dependencies
# named <project>_<tag> (slash -> underscore). premake ran from renderer/, so
# the clones live under renderer/dependencies — NOT the generic names used in
# earlier iterations (vulkan_headers/vulkan_memory_allocator).
set(RIVE_BOOTSTRAP_DEPS ${RIVE_ROOT}/renderer/dependencies)
file(GLOB VK_HEADERS_INC ${RIVE_BOOTSTRAP_DEPS}/KhronosGroup_Vulkan-Headers_*/include)
file(GLOB VMA_INC        ${RIVE_BOOTSTRAP_DEPS}/GPUOpen-LibrariesAndSDKs_VulkanMemoryAllocator_*/include)
if(NOT VK_HEADERS_INC OR NOT VMA_INC)
    message(FATAL_ERROR
        "Vulkan-Headers/VMA clones not found under ${RIVE_BOOTSTRAP_DEPS} — "
        "premake --with_vulkan must run before cmake (dependency.github clones them)")
endif()

# Static libs. build_rive.sh's premake sets targetdir(RIVE_BUILD_OUT) at config
# scope, so librive.a lands flat in RIVE_BUILD_OUT. The build script resolves
# them (find) and passes them explicitly; fall back to a search if absent.
if(NOT DEFINED RIVE_LIB OR NOT DEFINED RIVE_PLS_LIB)
    file(GLOB _rv  ${RIVE_BUILD}/librive.a ${RIVE_BUILD}/*/librive.a)
    file(GLOB _pls ${RIVE_BUILD}/librive_pls_renderer.a ${RIVE_BUILD}/*/librive_pls_renderer.a)
    list(LENGTH _rv _rvn)
    list(LENGTH _pls _plsn)
    if(_rvn EQUAL 0 OR _plsn EQUAL 0)
        message(FATAL_ERROR "No librive*.a under ${RIVE_BUILD} — run build_rive.sh first")
    endif()
    list(GET _rv  0 RIVE_LIB)
    list(GET _pls 0 RIVE_PLS_LIB)
endif()
message(STATUS "rive lib:      ${RIVE_LIB}")
message(STATUS "pls rend lib:  ${RIVE_PLS_LIB}")

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
    ${RIVE_RENDERER_INC}/../src   # bootstrap's #include "shaders/constants.glsl"
    ${RIVE_VKB_INC}
    ${RIVE_GEN_INC}
    ${VK_HEADERS_INC}
    ${VMA_INC}
)

# Same defines the rive premake sets when building with --with_vulkan
# (renderer/premake5_pls_renderer.lua): load all Vulkan entry points
# dynamically via the instance proc-addr (VK_NO_PROTOTYPES), and mark the
# Android platform so rive_vk_bootstrap's logging.hpp pulls <android/log.h>
# (otherwise LOG_ERROR_LINE expands with undeclared ANDROID_LOG_ERROR).
target_compile_definitions(inkalpha PRIVATE
    RIVE_VULKAN
    RIVE_ANDROID
    VK_NO_PROTOTYPES
    VMA_STATIC_VULKAN_FUNCTIONS=0
    VMA_DYNAMIC_VULKAN_FUNCTIONS=1
)

target_compile_options(inkalpha PRIVATE -Wall -Wextra)

# rive_pls_renderer (GPU) before rive (core); vulkan = libvulkan.so at runtime.
target_link_libraries(inkalpha PRIVATE
    ${RIVE_PLS_LIB}
    ${RIVE_LIB}
    vulkan
    android
    log
)