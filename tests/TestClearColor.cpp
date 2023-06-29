#include "TestClearColor.h"

#include "Core.h"

// extern
#include <imgui/imgui.h>


namespace tests {

TestClearColor::TestClearColor() : m_clearColor{ 0.1f, 0.3f, 0.5f, 1.0f } {}

TestClearColor::~TestClearColor() {}

void TestClearColor::onUpdate(float deltaTime) {}

void TestClearColor::onRender()
{
    GL_CALL(glClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]));
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
}

void TestClearColor::onImGuiRender()
{
    ImGui::Text("Set the OpenGL clear color: ");
    ImGui::Spacing();
    ImGui::ColorEdit4("Clear Color", m_clearColor);
    ImGui::Spacing();
}

}  // namespace tests
