add_executable(cube_shower
    main.cpp
    cube_data.h
    camera.h
    shader.h
   "mesh_data.h" "mesh_data.cpp")

target_compile_definitions(cube_shower PRIVATE _USE_MATH_DEFINES=1)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGLWidgets)
find_package(OpenMesh REQUIRED)

target_link_libraries(cube_shower PRIVATE 
    Qt6::Core 
    Qt6::Widgets 
    Qt6::OpenGLWidgets
    OpenMeshCore
    OpenMeshTools
    glfw
    libglew_static
)
if (WIN32)
    target_link_libraries(cube_shower PRIVATE OpenGL32)
endif()

add_custom_command(TARGET cube_shower POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_LIST_DIR}/model.obj"
    "$<TARGET_FILE_DIR:cube_shower>/model.obj"
)
add_custom_command(TARGET cube_shower POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_LIST_DIR}/Lowpoly_tree_sample.mtl"
    "$<TARGET_FILE_DIR:cube_shower>/Lowpoly_tree_sample.mtl"
)
