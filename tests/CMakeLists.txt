set(
  TEST_SOURCE_FILES
  Test.cpp
  TestClearColor.cpp
  TestCubes.cpp
  Test2DTransforms.cpp
  TestCamera.cpp
  TestModels.cpp
)

set(
  TEST_HEADER_FILES
  Test.h
  TestClearColor.h
  TestCubes.h
  Test2DTransforms.h
  TestCamera.h
  TestModels.h
)

add_library(tests STATIC ${TEST_SOURCE_FILES} ${TEST_HEADER_FILES})

set_target_properties(
  tests
  PROPERTIES  CXX_STANDARD 20
              CXX_STANDARD_REQUIRED YES
              CXX_EXTENSIONS NO
)

target_link_libraries(
  tests
  glad
  glm
  glfw
  imgui
  stb_image
  ${OPENGL_LIBRARIES}
)

target_include_directories(
    tests
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/extern/assimp/include
    ${CMAKE_BINARY_DIR}/extern/assimp/include
)
