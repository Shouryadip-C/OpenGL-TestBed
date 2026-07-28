#include "TestCuda.h"

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
    }

    m_currentSimulation = sim;

    // Initialize new simulation
    switch (m_currentSimulation) {
        case SimulationType::Forest: initForestSimulation(settings::windowWidth, settings::windowHeight, 8); break;
        case SimulationType::GameOfLife: initLifeSimulation(settings::windowWidth, settings::windowHeight, 8); break;
    }
}

TestCuda::TestCuda() : m_clearColor{ 0.1f, 0.3f, 0.5f, 1.0f }, m_time(0.0f)
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

    m_shader = std::make_unique<Shader>("../res/shader/fullscreen_quad.glsl");
    m_shader->bind();
    m_shader->setUniform1i("screenTexture", 0);

    // Fullscreen quad (NDC coordinates: -1 to 1)
    float quadVertices[] = { // positions     // texCoords
                             -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,  -1.0f, 1.0f, 0.0f,
                             1.0f,  1.0f,  1.0f, 1.0f, -1.0f, 1.0f,  0.0f, 1.0f
    };

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
    }
}

TestCuda::~TestCuda()
{
    switch (m_currentSimulation) {
        case SimulationType::Forest: cleanupForestSimulation(); break;

        case SimulationType::GameOfLife: cleanupLifeSimulation(); break;
    }
    CUDA_CHECK(cudaGraphicsUnregisterResource(m_cudaPboResource));
    GL_CALL(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
    GL_CALL(glDeleteBuffers(1, &m_pbo));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    GL_CALL(glDeleteTextures(1, &m_textureId));
}

void TestCuda::onUpdate(float deltaTime)
{
    m_time += 0.016f;

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
}

}  // namespace tests
