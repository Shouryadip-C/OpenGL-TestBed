#pragma once

#include <functional>
#include <string>
#include <vector>


namespace tests {

class Test
{
public:
    Test() {}
    virtual ~Test() {}

    virtual void onUpdate(float deltaTime) {}
    virtual void onRender() {}
    virtual void onImGuiRender() {}
};


class TestMenu: public Test
{
private:
    Test *&m_currentTest;

    std::vector<std::pair<std::string, std::function<Test *()>>> m_tests;

public:
    TestMenu(Test *&currentTestPointer);
    ~TestMenu();

    void onImGuiRender() override;

    template<typename T>
    void registerTest(const std::string &testName)
    {
        m_tests.push_back(std::make_pair(testName, []() { return new T(); }));
    }
};

}  // namespace tests
