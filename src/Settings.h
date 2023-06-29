#pragma once

#include <imgui/imgui.h>


namespace settings {

inline constexpr int              windowWidth{ 1200 };
inline constexpr int              windowHeight{ 800 };
inline constexpr ImGuiWindowFlags windowFlags{ ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize };

}