#pragma once

#include <imgui/imgui.h>


namespace settings {

inline constexpr int              windowWidth{ 1200 };
inline constexpr int              windowHeight{ 800 };
inline constexpr ImGuiWindowFlags windowFlags{};

namespace camera {
    inline constexpr float mouseSensitivity{ 0.1f };
    inline constexpr float movementSpeed{ 0.01f };
    inline constexpr float pitch{ 0.0f };
    inline constexpr float yaw{ -90.0f };
    inline constexpr float zoom{ 45.0f };
}  // namespace camera

}  // namespace settings
