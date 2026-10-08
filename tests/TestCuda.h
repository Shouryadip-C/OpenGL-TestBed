#pragma once

#include "Test.h"

// internal API
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "cuda_utils/Simulations.h"

// extern
#include "cuda_utils/CudaInterop.h"

#include <memory>


namespace tests {

enum class SimulationType { Forest, GameOfLife, LBM };

class TestCuda: public Test
{
private:
    float        m_clearColor[4];
    float        m_time;
    unsigned int m_pbo;
    unsigned int m_textureId;

    // LBM mouse state
    LBMMouseInput m_lbmMouse{};

    cudaGraphicsResource *m_cudaPboResource;

    std::unique_ptr<VertexBuffer> m_vertexBuffer;
    std::unique_ptr<IndexBuffer>  m_indexBuffer;
    std::unique_ptr<VertexArray>  m_VAO;
    std::unique_ptr<Shader>       m_shader;

    SimulationType m_currentSimulation = SimulationType::Forest;

public:
    TestCuda();
    ~TestCuda() override;

    void processMouseClick(GLFWwindow *window, int button, int action, int mods) override;
    void processMouseMovement(GLFWwindow *window, float xPos, float yPos) override;
    void processMouseScroll(GLFWwindow *window, float xPos, float yPos) override;
    void processInput(GLFWwindow *window, const float deltaTime) override;
    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onImGuiRender() override;

    void switchSimulation(SimulationType sim);
};

}  // namespace tests
