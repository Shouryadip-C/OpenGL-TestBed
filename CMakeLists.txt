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

# Add target subdirectories -------------------------------------------------- #
add_subdirectory(extern)
add_subdirectory(tests)
add_subdirectory(src)
