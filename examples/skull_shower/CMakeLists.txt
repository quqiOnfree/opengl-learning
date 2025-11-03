add_executable(skull_shower main.cpp)

target_compile_definitions(skull_shower PRIVATE _USE_MATH_DEFINES=1)
target_link_libraries(skull_shower PRIVATE
    glfw
    libglew_static
    Qt6::Core
    Qt6::Gui
    OpenMeshCore
    OpenMeshTools)
if (WIN32)
    target_link_libraries(skull_shower PRIVATE OpenGL32)
endif()

add_custom_command(TARGET skull_shower POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/examples/skull_shower/vertex.glsl"
    "$<TARGET_FILE_DIR:skull_shower>/vertex.glsl"
)
add_custom_command(TARGET skull_shower POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/examples/skull_shower/fragment.glsl"
    "$<TARGET_FILE_DIR:skull_shower>/fragment.glsl"
)
add_custom_command(TARGET skull_shower POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/examples/skull_shower/skull.stl"
    "$<TARGET_FILE_DIR:skull_shower>/skull.stl"
)
