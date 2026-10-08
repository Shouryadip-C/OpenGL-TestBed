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

if(TESTBED_ENABLE_CUDA)
  list(APPEND TEST_SOURCE_FILES TestCuda.cpp)
  list(APPEND TEST_HEADER_FILES TestCuda.h)
endif()

add_library(tests STATIC ${TEST_SOURCE_FILES} ${TEST_HEADER_FILES})

set_target_properties(
  tests
  PROPERTIES  CXX_STANDARD 20
              CXX_STANDARD_REQUIRED YES
              CUDA_RESOLVE_DEVICE_SYMBOLS ON
              CXX_EXTENSIONS NO
)

testbed_set_warnings(tests)

# DEBUG switches GL_CALL (src/Core.h) to its error-checking form
target_compile_definitions(tests PRIVATE $<$<CONFIG:Debug>:DEBUG>)

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

if(TESTBED_ENABLE_CUDA)
  target_link_libraries(
    tests
    $<TARGET_OBJECTS:cuda_utils>
    CUDA::cudart
    CUDA::cuda_driver
  )
  target_include_directories(tests PUBLIC ${CUDAToolkit_INCLUDE_DIRS})
  # lets the application know whether the cuda tests exist
  target_compile_definitions(tests PUBLIC TESTBED_HAS_CUDA)
endif()
