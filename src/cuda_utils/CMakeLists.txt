set(
  CUDA_SRC_FILES
  Simulations.cu
)

set(
  CUDA_HEADER_FILES
  Simulations.h
  CudaInterop.h
)

add_library(cuda_utils OBJECT ${CUDA_SRC_FILES} ${CUDA_HEADER_FILES})

target_compile_options(
  cuda_utils
  PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:-Wno-deprecated-gpu-targets>
)

set_target_properties(
  cuda_utils
  PROPERTIES
    CUDA_ARCHITECTURES 89
    CUDA_SEPARABLE_COMPILATION ON
    CUDA_RESOLVE_DEVICE_SYMBOLS ON
)

target_link_libraries(
  cuda_utils
  PRIVATE
    glad
    ${OPENGL_LIBRARIES}
)

target_include_directories(
  cuda_utils
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CUDAToolkit_INCLUDE_DIRS}
)
