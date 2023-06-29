#pragma once

#include "Test.h"

// internal API
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBuffer.h"

// extern
#include <glm/glm.hpp>

#include <memory>
#include <vector>


namespace tests {

class TestCamera: public Test
{
private:
    std::unique_ptr<VertexBuffer> m_vertexBuffer;
    std::unique_ptr<VertexArray>  m_VAO;
    std::unique_ptr<Shader>       m_shader;

    bool                                  m_mouseCaptured;
    bool                                  m_usePerspectiveProjection;
    float                                 m_visibilityRatio;
    double                                m_lastXPos;
    double                                m_lastYPos;
    double                                m_lastScroll;
    std::vector<glm::vec3>                m_cubePositions;
    glm::mat4                             m_proj2D;
    glm::mat4                             m_proj3D;
    glm::vec3                             m_rotationAxis;
    std::vector<std::unique_ptr<Texture>> m_textures;
    glm::mat4                             m_view;
    Camera                                m_camera;

public:
    TestCamera();
    ~TestCamera() override;

    void processMouseClick(GLFWwindow *window, int button, int action, int mods) override;
    void processMouseMovement(GLFWwindow *window, float xPos, float yPos) override;
    void processMouseScroll(GLFWwindow *window, float xPos, float yPos) override;
    void processInput(GLFWwindow *window, const float deltaTime) override;
    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onImGuiRender() override;
};

}  // namespace tests
