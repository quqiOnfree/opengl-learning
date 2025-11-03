add_executable(cube_shower
    main.c
    cube_data.h
    camera.h
    shader.h
)

target_compile_definitions(cube_shower PRIVATE _USE_MATH_DEFINES=1)
target_link_libraries(cube_shower PRIVATE
    glfw
    libglew_static
    glm
    stb
    Qt6::Core
    Qt6::Gui
    OpenMeshCore
    OpenMeshTools)
if (WIN32)
    target_link_libraries(cube_shower PRIVATE OpenGL32)
endif()
