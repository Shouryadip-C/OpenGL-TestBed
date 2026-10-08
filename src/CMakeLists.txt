if(TESTBED_ENABLE_CUDA)
  add_subdirectory(cuda_utils)
endif()

set(
  OPENGL_TESTBED_SRC_FILES
  main.cpp
  Assets.cpp
  Clock.cpp
  Core.cpp
  Renderer.cpp
  VertexArray.cpp
  VertexBuffer.cpp
  IndexBuffer.cpp
  Shader.cpp
  Texture.cpp
  Camera.cpp
  ScreenshotManager.cpp
  Mesh.cpp
  Model.cpp
)

set(
  OPENGL_TESTBED_HEADER_FILES
  Assets.h
  GpuConfig.h
  DebugTrap.h
  Clock.h
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
  Mesh.h
  Model.h
)

add_executable(opengl_testbed ${OPENGL_TESTBED_SRC_FILES} ${OPENGL_TESTBED_HEADER_FILES})

set_target_properties(
  opengl_testbed
  PROPERTIES  CXX_STANDARD 20
              CXX_STANDARD_REQUIRED YES
              CUDA_RESOLVE_DEVICE_SYMBOLS ON
              CXX_EXTENSIONS NO
)

testbed_set_warnings(opengl_testbed)

# DEBUG switches GL_CALL (src/Core.h) to its error-checking form
target_compile_definitions(opengl_testbed PRIVATE $<$<CONFIG:Debug>:DEBUG>)

target_link_libraries(
  opengl_testbed
  tests
  glad
  glm
  glfw
  imgui
  ImGuiFileDialog
  stb_image
  assimp
  ${OPENGL_LIBRARIES}
)

target_include_directories(
  opengl_testbed
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}/extern/assimp/include
    ${CMAKE_BINARY_DIR}/extern/assimp/include
)

if(TESTBED_ENABLE_CUDA)
  target_link_libraries(
    opengl_testbed
    $<TARGET_OBJECTS:cuda_utils>
    CUDA::cudart
    CUDA::cuda_driver
  )
  target_include_directories(opengl_testbed PRIVATE ${CUDAToolkit_INCLUDE_DIRS})
endif()

# *** Make the resources available next to the build
# The application looks for <build>/res and <build>/screenshots relative to the executable (src/Assets.h).
# res/ is symlinked so that edits to shaders and assets apply without rebuilding. Where symlinks are not
# available (Windows without developer mode) it is copied after every build instead.
set(TESTBED_RES_LINK "${CMAKE_BINARY_DIR}/res")
if(IS_DIRECTORY "${TESTBED_RES_LINK}" AND NOT IS_SYMLINK "${TESTBED_RES_LINK}")
  # left over copy from before res/ was symlinked
  file(REMOVE_RECURSE "${TESTBED_RES_LINK}")
endif()
file(CREATE_LINK "${CMAKE_SOURCE_DIR}/res" "${TESTBED_RES_LINK}" RESULT TESTBED_RES_LINK_RESULT SYMBOLIC)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/screenshots")

if(NOT TESTBED_RES_LINK_RESULT EQUAL 0)
  message(STATUS "Could not symlink res/ (${TESTBED_RES_LINK_RESULT}), copying it after each build instead")
  add_custom_command(
    TARGET opengl_testbed POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/res"
          "${TESTBED_RES_LINK}"
  )
endif()

# copying the imgui.ini file restores the layout of the imgui viewports previously used
add_custom_command(
  TARGET opengl_testbed POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/res/imgui.ini"
        $<TARGET_FILE_DIR:opengl_testbed>
)
