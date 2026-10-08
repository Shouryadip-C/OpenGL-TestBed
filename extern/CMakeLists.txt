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
# the vendored glm asks for cmake 3.2, which cmake >= 4.0 refuses to configure without this
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
add_subdirectory(glm)
unset(CMAKE_POLICY_VERSION_MINIMUM)

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
if(UNIX)
    find_package(X11 REQUIRED)
    if(X11_FOUND)
        target_include_directories(imgui PUBLIC ${X11_INCLUDE_DIR})
        target_link_libraries(imgui PUBLIC ${X11_LIBRARIES})
    endif()
endif()

target_link_libraries(imgui PUBLIC glfw)
target_include_directories(
  imgui
  PUBLIC  ${IMGUI_DIR}
          ${IMGUI_DIR}/backends
)

# GLM ------------------------------------------------------------------------ #
set(IMGUI_FILE_DIALOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiFileDialog)
add_library(ImGuiFileDialog
    ${IMGUI_FILE_DIALOG_DIR}/ImGuiFileDialog.cpp
    ${IMGUI_FILE_DIALOG_DIR}/ImGuiFileDialog.h
    ${IMGUI_FILE_DIALOG_DIR}/ImGuiFileDialogConfig.h
)

target_link_libraries(ImGuiFileDialog PUBLIC imgui)

# Use generator expressions for proper include directory handling
target_include_directories(ImGuiFileDialog
    PUBLIC
        $<BUILD_INTERFACE:${IMGUI_FILE_DIALOG_DIR}>
        $<INSTALL_INTERFACE:include>
)

if(UNIX)
    target_compile_options(ImGuiFileDialog PUBLIC -Wno-unknown-pragmas)
endif()

# ASSIMP --------------------------------------------------------------------- #
# its unit tests and install rules are not needed here and take a large part of the build time
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(assimp)

# Headers of the third-party targets count as system headers for everything that links them,
# so the warning flags of our own targets do not report issues inside them
set_target_properties(glad glfw glm stb_image imgui ImGuiFileDialog assimp PROPERTIES SYSTEM TRUE)
