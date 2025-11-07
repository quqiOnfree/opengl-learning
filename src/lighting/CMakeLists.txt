add_executable(lighting main.cpp)

target_compile_definitions(lighting PRIVATE _USE_MATH_DEFINES=1)
target_include_directories(lighting PRIVATE "${CMAKE_CURRENT_LIST_DIR}/include")
target_link_libraries(lighting PRIVATE
    glfw
    libglew_static
    glm
    Qt6::Core
    Qt6::Gui
    OpenMeshCore
    OpenMeshTools)
if (WIN32)
    target_link_libraries(lighting PRIVATE OpenGL32)
endif()

add_custom_command(TARGET lighting POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_LIST_DIR}/vertex.glsl"
    "$<TARGET_FILE_DIR:lighting>/vertex.glsl"
)
add_custom_command(TARGET lighting POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_LIST_DIR}/fragment.glsl"
    "$<TARGET_FILE_DIR:lighting>/fragment.glsl"
)
add_custom_command(TARGET lighting POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_LIST_DIR}/light_fragment.glsl"
    "$<TARGET_FILE_DIR:lighting>/light_fragment.glsl"
)
add_custom_command(TARGET lighting POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_LIST_DIR}/cube.stl"
    "$<TARGET_FILE_DIR:lighting>/cube.stl"
)
