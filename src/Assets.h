#pragma once

#include <filesystem>


// Locates files relative to the executable instead of the working directory,
// so the application can be started from anywhere.
//
// Layout next to the build:
//   <root>/src/opengl_testbed    the executable
//   <root>/res/                  shaders, textures, models
//   <root>/screenshots/
namespace assets {

// Directory that contains the executable
const std::filesystem::path &executableDir();

// Parent of the executable's directory, holds res/ and screenshots/
const std::filesystem::path &rootDir();

// Absolute path of a file inside res/, e.g. assets::path("shader/cube_shader.glsl")
std::filesystem::path path(const std::filesystem::path &relativePath);

}  // namespace assets
