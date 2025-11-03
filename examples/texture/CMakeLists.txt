add_executable(texture main.cpp)

target_compile_definitions(texture PRIVATE _USE_MATH_DEFINES=1)
target_link_libraries(texture PRIVATE
    glfw
    libglew_static
    glm
    stb
    Qt6::Core
    Qt6::Gui
    OpenMeshCore
    OpenMeshTools)
if (WIN32)
    target_link_libraries(texture PRIVATE OpenGL32)
endif()
