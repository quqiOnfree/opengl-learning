cmake_minimum_required(VERSION 3.24)

project(opengl-learning)

set(glew-cmake_BUILD_SHARED OFF)
set(glew-cmake_BUILD_STATIC ON)

find_package(OpenGL REQUIRED)

add_subdirectory(glfw)
add_subdirectory(glew-cmake)

add_executable(opengl-learning main.cpp)
target_link_libraries(opengl-learning PRIVATE glfw libglew_static)
if (WIN32)
    target_link_libraries(opengl-learning PRIVATE OpenGL32)
endif()
