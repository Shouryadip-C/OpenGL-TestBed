#pragma once

#include "Test.h"

// internal API
#include "IndexBuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

// extern
#include <glm/glm.hpp>

#include <memory>
#include <vector>


namespace tests {

class Test2DTransforms: public Test
{
private:
    std::unique_ptr<VertexBuffer> m_vertexBuffer;
    std::unique_ptr<IndexBuffer>  m_indexBuffer;
    std::unique_ptr<VertexArray>  m_VAO;
    std::unique_ptr<Shader>       m_shader;

    int                                   m_triangleNumber;
    bool                                  m_usePerspectiveProjection;
    float                                 m_viewAngle;
    float                                 m_viewBoundaries[4];
    float                                 m_visibilityRatio;
    glm::mat4                             m_proj3D;
    glm::vec3                             m_rotationAxis;
    std::vector<std::unique_ptr<Texture>> m_textures;

public:
    Test2DTransforms();
    ~Test2DTransforms() override;

    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onImGuiRender() override;
};

}  // namespace tests
