add_executable(robotic_car main.cpp)

target_compile_definitions(robotic_car PRIVATE
    _USE_MATH_DEFINES=1
    STB_IMAGE_IMPLEMENTATION=1)
target_include_directories(robotic_car PRIVATE include)
target_link_libraries(robotic_car PRIVATE
    glfw
    libglew_static
    stb
    glm
    OpenMeshCore
    OpenMeshTools
    freetype)
if (WIN32)
    target_link_libraries(robotic_car PRIVATE OpenGL32)
endif()

add_custom_command(TARGET robotic_car POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_LIST_DIR}/line.jpg"
    "$<TARGET_FILE_DIR:robotic_car>/line.jpg"
)
