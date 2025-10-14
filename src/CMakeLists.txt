set(
  OPENGL_TESTBED_SRC_FILES
  main.cpp
  Core.cpp
  Renderer.cpp
  VertexArray.cpp
  VertexBuffer.cpp
  IndexBuffer.cpp
  Shader.cpp
  Texture.cpp
  Camera.cpp
  ScreenshotManager.cpp
)

set(
  OPENGL_TESTBED_HEADER_FILES
  DebugTrap.h
  Core.h
  Settings.h
  Glfw.h
  Renderer.h
  VertexArray.h
  VertexBuffer.h
  VertexBufferLayout.h
  IndexBuffer.h
  Shader.h
  Texture.h
  Camera.h
  ScreenshotManager.h
)

add_executable(opengl_testbed ${OPENGL_TESTBED_SRC_FILES} ${OPENGL_TESTBED_HEADER_FILES})

set_target_properties(
  opengl_testbed
  PROPERTIES  CXX_STANDARD 20
              CXX_STANDARD_REQUIRED YES
              CXX_EXTENSIONS NO
)

target_link_libraries(
  opengl_testbed
  tests
  glad
  glm
  glfw
  imgui
  stb_image
  ${OPENGL_LIBRARIES}
)

target_include_directories(
  opengl_testbed
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# *** Copy resources directory to the build directory
# copying the imgui.ini file restores the layout of the imgui viewports previously used
add_custom_command(
  TARGET opengl_testbed POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/res/imgui.ini"
        $<TARGET_FILE_DIR:opengl_testbed>
  COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/res"
        $<TARGET_FILE_DIR:opengl_testbed>/../res
  COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:opengl_testbed>/../screenshots
)
