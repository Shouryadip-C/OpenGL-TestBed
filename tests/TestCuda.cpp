#include "TestCuda.h"

#include "Assets.h"
#include "Core.h"
#include "Renderer.h"
#include "Settings.h"
#include "cuda_utils/Simulations.h"

// extern
#include <imgui/imgui.h>

#include <iostream>


namespace tests {

void TestCuda::switchSimulation(SimulationType sim)
{
    // Cleanup current simulation
    switch (m_currentSimulation) {
        case SimulationType::Forest: cleanupForestSimulation(); break;
        case SimulationType::GameOfLife: cleanupLifeSimulation(); break;
        case SimulationType::LBM: cleanupLBMSimulation(); break;
    }

    m_currentSimulation = sim;

    // Reset LBM input state so we don't carry a mouse drag
    // from one simulation into another.
    m_lbmMouse = {};

    // Initialize new simulation
    switch (m_currentSimulation) {
        case SimulationType::Forest: initForestSimulation(settings::windowWidth, settings::windowHeight, 8); break;
        case SimulationType::GameOfLife: initLifeSimulation(settings::windowWidth, settings::windowHeight, 8); break;
        case SimulationType::LBM: initLBMSimulation(settings::windowWidth, settings::windowHeight, 4); break;
    }
}


TestCuda::TestCuda()
  : m_clearColor{ 0.1f, 0.3f, 0.5f, 1.0f }, m_time(0.0f), m_pbo(0), m_textureId(0), m_cudaPboResource(nullptr)
{
    GL_CALL(glGenBuffers(1, &m_pbo));
    GL_CALL(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo));
    GL_CALL(glBufferData(GL_PIXEL_UNPACK_BUFFER, settings::windowWidth * settings::windowHeight * 4, nullptr,
                         GL_DYNAMIC_DRAW));
    GL_CALL(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));

    // Register buffer with CUDA
    CUDA_CHECK(cudaGraphicsGLRegisterBuffer(&m_cudaPboResource, m_pbo, cudaGraphicsMapFlagsWriteDiscard));

    // Create texture for rendering
    GL_CALL(glActiveTexture(GL_TEXTURE0));
    GL_CALL(glGenTextures(1, &m_textureId));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, m_textureId));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, settings::windowWidth, settings::windowHeight, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, nullptr));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    m_shader = std::make_unique<Shader>(assets::path("shader/fullscreen_quad.glsl"));
    m_shader->bind();
    m_shader->setUniform1i("screenTexture", 0);

    // clang-format off
    // Fullscreen quad (NDC coordinates: -1 to 1)
    float quadVertices[] = {
        // positions  // texCoords
        -1.0f, -1.0f, 0.0f, 1.0f,
        1.0f,  -1.0f, 1.0f, 1.0f,
        1.0f,  1.0f,  1.0f, 0.0f,
        -1.0f, 1.0f,  0.0f, 0.0f
    };
    // clang-format on

    unsigned int quadIndices[] = { 0, 1, 2, 2, 3, 0 };

    m_VAO = std::make_unique<VertexArray>();
    unsigned int buffSize{ 4 * 4 * sizeof(float) };
    m_vertexBuffer = std::make_unique<VertexBuffer>(quadVertices, buffSize);
    m_indexBuffer  = std::make_unique<IndexBuffer>(quadIndices, 6);

    VertexBufferLayout layout;
    layout.push<float>(2);  // vertex position
    layout.push<float>(2);  // tex coords
    m_VAO->addBuffer(*m_vertexBuffer, layout);
    m_VAO->addBuffer(*m_indexBuffer);

    // Initialize new simulation
    switch (m_currentSimulation) {
        case SimulationType::Forest: initForestSimulation(settings::windowWidth, settings::windowHeight, 8); break;
        case SimulationType::GameOfLife: initLifeSimulation(settings::windowWidth, settings::windowHeight, 8); break;
        case SimulationType::LBM: initLBMSimulation(settings::windowWidth, settings::windowHeight, 4); break;
    }
}

TestCuda::~TestCuda()
{
    switch (m_currentSimulation) {
        case SimulationType::Forest: cleanupForestSimulation(); break;
        case SimulationType::GameOfLife: cleanupLifeSimulation(); break;
        case SimulationType::LBM: cleanupLBMSimulation(); break;
    }
    CUDA_CHECK(cudaGraphicsUnregisterResource(m_cudaPboResource));
    GL_CALL(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
    GL_CALL(glDeleteBuffers(1, &m_pbo));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    GL_CALL(glDeleteTextures(1, &m_textureId));
}

void TestCuda::onUpdate(float deltaTime)
{
    m_time += deltaTime;

    uint32_t *devPtr = nullptr;
    size_t    size   = 0;

    // Map PBO for CUDA access
    CUDA_CHECK(cudaGraphicsMapResources(1, &m_cudaPboResource, 0));
    CUDA_CHECK(cudaGraphicsResourceGetMappedPointer((void **)&devPtr, &size, m_cudaPboResource));

    // Run CUDA kernel
    switch (m_currentSimulation) {
        case SimulationType::Forest:
            runForestSimulationStep(devPtr, settings::windowWidth, settings::windowHeight, m_time);
            break;
        case SimulationType::GameOfLife:
            runLifeSimulationStep(devPtr, settings::windowWidth, settings::windowHeight, m_time);
            break;
        case SimulationType::LBM:
            runLBMSimulationStep(devPtr, settings::windowWidth, settings::windowHeight, m_time, m_lbmMouse);
            break;
    }

    // Unmap CUDA resource so OpenGL can use it
    CUDA_CHECK(cudaGraphicsUnmapResources(1, &m_cudaPboResource, 0));
}

void TestCuda::onRender()
{
    Renderer::setClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]);
    Renderer::clear();

    GL_CALL(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, m_textureId));
    GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, settings::windowWidth, settings::windowHeight, GL_RGBA,
                            GL_UNSIGNED_BYTE, 0));
    GL_CALL(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));

    Renderer::draw(*m_VAO, *m_shader);
}

void TestCuda::onImGuiRender()
{
    ImGui::Text("Set the OpenGL clear color: ");
    ImGui::Spacing();
    ImGui::ColorEdit4("Clear Color", m_clearColor);
    ImGui::Spacing();

    ImGui::Separator();
    ImGui::Text("Simulations");

    if (ImGui::Button("Forest Fire", ImVec2(-FLT_MIN, 1.2f * ImGui::GetTextLineHeightWithSpacing()))) {
        switchSimulation(SimulationType::Forest);
    }
    if (ImGui::Button("Game of Life", ImVec2(-FLT_MIN, 1.2f * ImGui::GetTextLineHeightWithSpacing()))) {
        switchSimulation(SimulationType::GameOfLife);
    }
    if (ImGui::Button("Lattice Boltzmann", ImVec2(-FLT_MIN, 1.2f * ImGui::GetTextLineHeightWithSpacing()))) {
        switchSimulation(SimulationType::LBM);
    }

    if (m_currentSimulation == SimulationType::LBM) {
        ImGui::Separator();
        ImGui::Text("LBM Controls");
        ImGui::Text("Left Drag   : Stir fluid");
        ImGui::Text("Right Drag  : Add obstacle");
        ImGui::Text("Middle Drag : Remove obstacle");
        ImGui::Text("Color       : Velocity magnitude");
    }
}

void TestCuda::processMouseClick(GLFWwindow *window, int button, int action, int mods)
{
    if (m_currentSimulation != SimulationType::LBM) {
        return;
    }

    if (action != GLFW_PRESS && action != GLFW_RELEASE) {
        return;
    }

    bool pressed = (action == GLFW_PRESS);

    if (pressed) {
        m_lbmMouse.previousX = m_lbmMouse.x;
        m_lbmMouse.previousY = m_lbmMouse.y;
    }

    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT: m_lbmMouse.left = pressed; break;
        case GLFW_MOUSE_BUTTON_RIGHT: m_lbmMouse.right = pressed; break;
        case GLFW_MOUSE_BUTTON_MIDDLE: m_lbmMouse.middle = pressed; break;
        default: break;
    }
}

// Mouse movement
void TestCuda::processMouseMovement(GLFWwindow *window, float xPos, float yPos)
{
    if (m_currentSimulation != SimulationType::LBM) {
        return;
    }

    m_lbmMouse.previousX = m_lbmMouse.x;
    m_lbmMouse.previousY = m_lbmMouse.y;
    m_lbmMouse.x         = xPos;
    m_lbmMouse.y         = yPos;
}

// Mouse scroll
void TestCuda::processMouseScroll(GLFWwindow *window, float xPos, float yPos)
{
    if (m_currentSimulation != SimulationType::LBM) {
        return;
    }

    // Nothing for now.
    //
    // Later this can control:
    //     - brush radius
    //     - injection strength
    //     - visualization scale
}

// Keyboard input
void TestCuda::processInput(GLFWwindow *window, const float deltaTime)
{
    if (window == nullptr) {
        return;
    }

    // Reserved for future LBM controls.
    if (m_currentSimulation == SimulationType::LBM) {
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            // Reset LBM later.
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            // Pause/resume later.
        }
    }
}

}  // namespace tests
