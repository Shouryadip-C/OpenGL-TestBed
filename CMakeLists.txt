cmake_minimum_required(VERSION 3.25)

# Set the project name and language ------------------------------------------ #
project(OpenGL_TestBed LANGUAGES CXX C)
# Define Languages CXX and C for dependencies otherwise when using GCC compiler error is generated
# CMake Error: Error required internal CMake variable not set, cmake may not be built correctly.
# Missing variable is: CMAKE_C_COMPILE_OBJECT

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory
       (called a build directory) and run CMake from there."
  )
endif()

# CUDA (optional) ------------------------------------------------------------ #
# Enabled by default only when a working CUDA compiler is found, so the project still builds
# on machines without an NVIDIA GPU. Force it with -DTESTBED_ENABLE_CUDA=ON/OFF.
#
# nvcc only supports specific host compiler versions. If the system default is too new, point it
# at a supported one when configuring, e.g. -DCMAKE_CUDA_HOST_COMPILER=/usr/bin/g++-13
# (or the CUDAHOSTCXX environment variable).
include(CheckLanguage)
if(CMAKE_CUDA_HOST_COMPILER)
  # check_language() runs in a separate cmake process that only sees the environment variable
  set(ENV{CUDAHOSTCXX} "${CMAKE_CUDA_HOST_COMPILER}")
endif()
check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
  set(TESTBED_CUDA_DEFAULT ON)
else()
  set(TESTBED_CUDA_DEFAULT OFF)
endif()
option(TESTBED_ENABLE_CUDA "Build the CUDA simulations" ${TESTBED_CUDA_DEFAULT})

if(TESTBED_ENABLE_CUDA)
  enable_language(CUDA)
  find_package(CUDAToolkit REQUIRED)
  set(CMAKE_CUDA_ARCHITECTURES "native")
endif()
message(STATUS "CUDA simulations: ${TESTBED_ENABLE_CUDA}")
if(NOT CMAKE_CUDA_COMPILER)
  find_program(TESTBED_NVCC nvcc)
  if(TESTBED_NVCC)
    # nvcc is installed but could not compile a test file, which is easy to miss
    message(
      WARNING
        "Found ${TESTBED_NVCC} but it failed the compiler check, so the CUDA simulations are disabled. "
        "See 'Looking for a CUDA compiler' in ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeConfigureLog.yaml"
    )
  endif()
endif()

# Add target subdirectories -------------------------------------------------- #
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(Warnings)

add_subdirectory(extern)
add_subdirectory(tests)
add_subdirectory(src)
