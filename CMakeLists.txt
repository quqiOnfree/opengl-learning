cmake_minimum_required(VERSION 3.24)

project(opengl-learning)

set(glew-cmake_BUILD_SHARED OFF)
set(glew-cmake_BUILD_STATIC ON)

find_package(Qt6 COMPONENTS Core Gui REQUIRED)

add_subdirectory(glfw)
add_subdirectory(glew-cmake)
add_subdirectory(OpenMesh)

add_executable(opengl-learning main.cpp)
target_compile_definitions(opengl-learning PRIVATE _USE_MATH_DEFINES=1)
target_link_libraries(opengl-learning PRIVATE
    glfw
    libglew_static
    Qt6::Core
    Qt6::Gui
    OpenMeshCore
    OpenMeshTools)
if (WIN32)
    target_link_libraries(opengl-learning PRIVATE OpenGL32)
endif()
