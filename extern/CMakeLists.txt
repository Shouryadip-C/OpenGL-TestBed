# Some magic required on linux btw if you're working with OpenGL
find_package(OpenGL REQUIRED)

# GLAD ----------------------------------------------------------------------- #
add_subdirectory(glad_core_43)

# using the debug version of glad gives debug output on errors
# this bypasses any debug procedures like glGetEror() and glDebugMessageCallback()
# you wont be able to implement your own debug methods
# add_subdirectory(glad_core_43_debug)

# GLFW ----------------------------------------------------------------------- #
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw)

# STB Image ------------------------------------------------------------------ #
add_subdirectory(stb_image)

# GLM ------------------------------------------------------------------------ #
add_subdirectory(glm)

# IMGUI ---------------------------------------------------------------------- #
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
set(
  IMGUI_SOURCE_FILES
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_demo.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp

  # backends
  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

set(
  IMGUI_HEADER_FILES
  ${IMGUI_DIR}/imgui.h
  ${IMGUI_DIR}/imconfig.h
  ${IMGUI_DIR}/imgui_internal.h
  ${IMGUI_DIR}/imstb_rectpack.h
  ${IMGUI_DIR}/imstb_textedit.h
  ${IMGUI_DIR}/imstb_truetype.h

  # backends
  ${IMGUI_DIR}/backends/imgui_impl_glfw.h
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.h
)

add_library(imgui STATIC ${IMGUI_SOURCE_FILES} ${IMGUI_HEADER_FILES})
target_link_libraries(imgui glfw)
target_include_directories(
  imgui
  PUBLIC  ${IMGUI_DIR}
          ${IMGUI_DIR}/backends
)


