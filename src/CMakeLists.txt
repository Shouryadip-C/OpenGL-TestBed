set(
  OPENGL_TESTBED_SRC_FILES
  main.cpp
  Core.cpp
)

set(
  OPENGL_TESTBED_HEADER_FILES
  DebugTrap.h
  Core.h
  Settings.h
  Glfw.h
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

