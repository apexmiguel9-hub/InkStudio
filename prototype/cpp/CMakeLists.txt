# InkAlpha native module — ThorVG GL canvas + ported Inkscape tools.
cmake_minimum_required(VERSION 3.22)
project(InkAlphaNative LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED THORVG_INC OR NOT DEFINED THORVG_LIB)
    message(FATAL_ERROR "Pass -DTHORVG_INC=<thorvg>/inc and -DTHORVG_LIB=<path>/libthorvg.a (meson static build, engines=cpu,gl)")
endif()

add_library(inkalpha SHARED
    jni_bridge.cpp
    renderer.cpp
    tool/rect_tool_port.cpp
    tool/select_tool_port.cpp
    tool/node_tool_port.cpp
    sham/tool_base.cpp
    sham/snap.cpp
    sham/desktop.cpp
    sham/seltrans.cpp
)

target_include_directories(inkalpha PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/sham
    ${CMAKE_CURRENT_SOURCE_DIR}/tool
    ${THORVG_INC}
)

target_compile_options(inkalpha PRIVATE -Wall -Wextra)

target_link_libraries(inkalpha PRIVATE
    ${THORVG_LIB}
    GLESv2
    EGL
    android
    log
)