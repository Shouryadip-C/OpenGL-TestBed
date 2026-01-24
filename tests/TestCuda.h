#pragma once

#include "Test.h"

// internal API
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBuffer.h"

// extern
#include "cuda_utils/CudaInterop.h"

#include <memory>


namespace tests {

class TestCuda: public Test
{
private:
    float        m_clearColor[4];
    float        m_time;
    unsigned int m_pbo;
    unsigned int m_textureId;

    cudaGraphicsResource *m_cudaPboResource;

    std::unique_ptr<VertexBuffer> m_vertexBuffer;
    std::unique_ptr<IndexBuffer>  m_indexBuffer;
    std::unique_ptr<VertexArray>  m_VAO;
    std::unique_ptr<Shader>       m_shader;

public:
    TestCuda();
    ~TestCuda() override;

    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onImGuiRender() override;
};

}  // namespace tests
