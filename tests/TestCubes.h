#pragma once

#include "Test.h"

// internal API
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBuffer.h"

// extern
#include <glm/glm.hpp>

#include <memory>
#include <vector>


namespace tests {

class TestCubes: public Test
{
private:
    std::unique_ptr<VertexBuffer> m_vertexBuffer;
    std::unique_ptr<VertexArray>  m_VAO;
    std::unique_ptr<Shader>       m_shader;

    bool                                  m_usePerspectiveProjection;
    float                                 m_viewAngle;
    float                                 m_visibilityRatio;
    glm::vec3                             m_cameraPos;
    std::vector<glm::vec3>                m_cubePositions;
    glm::mat4                             m_proj2D;
    glm::mat4                             m_proj3D;
    glm::vec3                             m_rotationAxis;
    std::vector<std::unique_ptr<Texture>> m_textures;
    glm::mat4                             m_view;

public:
    TestCubes();
    ~TestCubes() override;

    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onImGuiRender() override;
};

}  // namespace tests
