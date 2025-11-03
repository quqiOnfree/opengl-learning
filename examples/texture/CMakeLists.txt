add_executable(texture main.cpp)

target_compile_definitions(texture PRIVATE
    _USE_MATH_DEFINES=1
    STB_IMAGE_IMPLEMENTATION=1)
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

add_custom_command(TARGET texture POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/examples/texture/vertex.glsl"
    "$<TARGET_FILE_DIR:texture>/vertex.glsl"
)
add_custom_command(TARGET texture POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/examples/texture/fragment.glsl"
    "$<TARGET_FILE_DIR:texture>/fragment.glsl"
)
add_custom_command(TARGET texture POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/examples/texture/container.jpeg"
    "$<TARGET_FILE_DIR:texture>/container.jpeg"
)
