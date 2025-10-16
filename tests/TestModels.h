#pragma once

#include "Test.h"

// internal API
#include "Camera.h"
#include "Model.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBuffer.h"

// extern
#include <glm/glm.hpp>

#include <memory>
#include <vector>


namespace tests {

class TestModels: public Test
{
private:
    std::unique_ptr<Shader> m_shader;
    std::unique_ptr<Model>  m_model;

    bool                                  m_mouseCaptured;
    bool                                  m_usePerspectiveProjection;
    double                                m_lastXPos;
    double                                m_lastYPos;
    double                                m_lastScroll;
    glm::mat4                             m_proj2D;
    glm::mat4                             m_proj3D;
    std::vector<std::unique_ptr<Texture>> m_textures;
    glm::mat4                             m_view;
    Camera                                m_camera;

public:
    TestModels();
    ~TestModels() override;

    void processMouseClick(GLFWwindow *window, int button, int action, int mods) override;
    void processMouseMovement(GLFWwindow *window, float xPos, float yPos) override;
    void processMouseScroll(GLFWwindow *window, float xPos, float yPos) override;
    void processInput(GLFWwindow *window, const float deltaTime) override;
    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onImGuiRender() override;
};

}  // namespace tests
