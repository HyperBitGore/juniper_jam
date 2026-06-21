cmake_minimum_required(VERSION 3.21)
project(juniper_jam)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch g_engine_2d
FetchContent_Declare(
    g_engine_2d
    GIT_REPOSITORY https://github.com/HyperBitGore/g_engine_2d.git
    GIT_TAG main
)

FetchContent_GetProperties(g_engine_2d)
if(NOT g_engine_2d_POPULATED)
    FetchContent_Populate(g_engine_2d)
endif()

# g_engine source files
set(ENGINE_SOURCES
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/g_engine_2d.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/audio/audio_player.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/engine_modern.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/engine_start.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/rendering/font_renderer.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/function_loader.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/gl_defines.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/util/hashmap.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/img_loading/image_loader.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/input.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/util/matrix.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/util/matrix.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/util/shader.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/window.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/audio/audio.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/backend.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/util/vector.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/util/vector.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/rendering/primitive_renderer.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/rendering/primitive_renderer_shader.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/rendering/image_renderer.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/img_loading/image_atlas.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/img_loading/bmp_loader.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/util/shader.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/img_loading/image_loader.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/rendering/image_renderer.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/rendering/image_renderer_shader.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/rendering/primitive_renderer.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/rendering/font_renderer.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/file_loading/font_loader.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/file_loading/font_loader.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/img_loading/png_loader.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/img_loading/common.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/img_loading/inflate.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/file_loading/file_reader.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/file_loading/file_reader.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/util/logging.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/rendering/renderer.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/rendering/wireframe_renderer.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/rendering/wireframe_renderer.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/rendering/three_dee_renderer.hpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/rendering/three_dee_renderer.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/file_loading/model_loading/obj_loader.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/file_loading/model_loading/gltf_loader.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/file_loading/model_loading/model.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/rendering/camera.hpp
)

# Create the executable
add_executable(juniper_jam src/main.cpp src/game.cpp src/menu.cpp src/game_loop.cpp
    src/spatial_hashmap.cpp
${ENGINE_SOURCES})

# Include g_engine_2d headers
target_include_directories(juniper_jam PRIVATE ${g_engine_2d_SOURCE_DIR}/include)

if(WIN32)
    target_link_libraries(juniper_jam gdi32 user32 opengl32 ole32)
    target_compile_options(juniper_jam PRIVATE /utf-8)
endif(WIN32)
if(UNIX)
find_package(X11 REQUIRED)
find_package(ALSA REQUIRED)
find_package(OpenGL REQUIRED)
message(STATUS "X11_FOUND: ${X11_FOUND}")
message(STATUS "X11_INCLUDE_DIR: ${X11_INCLUDE_DIR}")
message(STATUS "X11_LIBRARIES: ${X11_LIBRARIES}")
message(STATUS "X11_Xkblib_FOUND: ${X11_Xkb_FOUND}")
message(STATUS "X11_Xkblib_LIB: ${X11_Xkb_INCLUDE_PATH}")
if (NOT X11_Xkb_FOUND)
    message(FATAL_ERROR "XKB extension not found")
endif()
target_include_directories(juniper_jam PRIVATE ${X11_INCLUDE_DIR} ${X11_Xkb_INCLUDE_PATH})
target_link_libraries(juniper_jam OpenGL::GL dl X11::X11 X11::Xkb ALSA::ALSA)
endif(UNIX)
target_include_directories(juniper_jam PUBLIC include/GL)
