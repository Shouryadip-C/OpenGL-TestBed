#pragma once

#include "Test.h"

namespace tests {

class TestClearColor: public Test
{
private:
    float m_clearColor[4];

public:
    TestClearColor();
    ~TestClearColor() override;

    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onImGuiRender() override;
};

}  // namespace tests
