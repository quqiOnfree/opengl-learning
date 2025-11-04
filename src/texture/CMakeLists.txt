add_executable(texture main.cpp)

target_compile_definitions(texture PRIVATE
    _USE_MATH_DEFINES=1
    STB_IMAGE_IMPLEMENTATION=1)
target_link_libraries(texture PRIVATE
    glfw
    libglew_static
    stb
    glm)
if (WIN32)
    target_link_libraries(texture PRIVATE OpenGL32)
endif()

add_custom_command(TARGET texture POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_LIST_DIR}/vertex.glsl"
    "$<TARGET_FILE_DIR:texture>/vertex.glsl"
)
add_custom_command(TARGET texture POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_LIST_DIR}/fragment.glsl"
    "$<TARGET_FILE_DIR:texture>/fragment.glsl"
)
add_custom_command(TARGET texture POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_LIST_DIR}/container.jpeg"
    "$<TARGET_FILE_DIR:texture>/container.jpeg"
)
