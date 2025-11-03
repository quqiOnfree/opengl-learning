cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(opengl-learning)

set(glew-cmake_BUILD_SHARED OFF)
set(glew-cmake_BUILD_STATIC ON)
set(GLM_ENABLE_CXX_20 ON)

find_package(Qt6 COMPONENTS Core Gui REQUIRED)

add_subdirectory(glfw)
add_subdirectory(glew-cmake)
add_subdirectory(OpenMesh)
add_subdirectory(glm)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)
target_compile_definitions(stb INTERFACE STB_IMAGE_IMPLEMENTATION=1)

add_subdirectory(src/skull_shower)
add_subdirectory(src/texture)
add_subdirectory(src/cube_shower)
