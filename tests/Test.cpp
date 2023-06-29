#include "Test.h"

#include <imgui/imgui.h>

namespace tests {

TestMenu::TestMenu(Test *&currentTestPointer) : m_currentTest(currentTestPointer) {}

TestMenu::~TestMenu() {}

void TestMenu::onImGuiRender()
{
    for (auto &test: m_tests) {
        if (ImGui::Button(test.first.c_str(), ImVec2(-FLT_MIN, 1.2 * ImGui::GetTextLineHeightWithSpacing()))) {
            m_currentTest = test.second();
        }
        ImGui::Spacing();
    }
}

}  // namespace tests
