#include "TestCubes.h"

#include "Assets.h"
#include "Core.h"
#include "Glfw.h"
#include "Renderer.h"
#include "Settings.h"
#include "VertexBufferLayout.h"

// external
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>


namespace tests {

TestCubes::TestCubes()
  : m_usePerspectiveProjection(true),
    m_viewAngle(60.0f),
    m_visibilityRatio(0.0f),
    m_cameraPos(glm::vec3(0.0f, 0.0f, -3.0f)),
    m_cubePositions{ glm::vec3(0.0f, 0.0f, 0.0f),     glm::vec3(2.0f, 5.0f, -15.0f), glm::vec3(-1.5f, -2.2f, -2.5f),
                     glm::vec3(-3.8f, -2.0f, -12.3f), glm::vec3(2.4f, -0.4f, -3.5f), glm::vec3(-1.7f, 3.0f, -7.5f),
                     glm::vec3(1.3f, -2.0f, -2.5f),   glm::vec3(1.5f, 2.0f, -2.5f),  glm::vec3(1.5f, 0.2f, -1.5f),
                     glm::vec3(-1.3f, 1.0f, -1.5f) },
    m_proj2D(glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.0f, 20.0f)),
    m_proj3D(glm::perspective(glm::radians(m_viewAngle),
                              (float)settings::windowWidth / (float)settings::windowHeight,
                              0.1f,
                              100.0f)),
    m_rotationAxis(glm::vec3(0.5f, 1.0f, 0.0f)),
    m_view(glm::translate(glm::mat4(1.0f), m_cameraPos))
{
    // TODO: check if texture binding problems occur on copy construction of Texture object
    m_textures.push_back(std::make_unique<Texture>(assets::path("textures/awesomeface.png")));
    m_textures.push_back(std::make_unique<Texture>(assets::path("textures/hells_paradise.jpg")));

    for (unsigned int i = 0; i < m_textures.size(); i++) {
        m_textures[i]->bind(i);
    }

    m_shader = std::make_unique<Shader>(assets::path("shader/cube_shader.glsl"));
    m_shader->bind();
    m_shader->setUniform1i("u_texture1", 0);
    m_shader->setUniform1i("u_texture2", 1);

    float cubeVertices[]{ -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.5f,  0.5f,  -0.5f,
                          1.0f,  1.0f,  0.5f,  0.5f,  -0.5f, 1.0f,  1.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  -0.5f,
                          -0.5f, -0.5f, 0.0f,  0.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  0.5f,  -0.5f, 0.5f,  1.0f,
                          0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  1.0f,  1.0f,  -0.5f, 0.5f,
                          0.5f,  0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  1.0f,  0.0f,
                          -0.5f, 0.5f,  -0.5f, 1.0f,  1.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  1.0f,  -0.5f, -0.5f, -0.5f,
                          0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  1.0f,  0.0f,  0.5f,
                          0.5f,  0.5f,  1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 1.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 0.0f,
                          1.0f,  0.5f,  -0.5f, -0.5f, 0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  0.0f,  0.5f,  0.5f,
                          0.5f,  1.0f,  0.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  1.0f,
                          0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  -0.5f, -0.5f, 0.5f,
                          0.0f,  0.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  1.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.5f,
                          0.5f,  -0.5f, 1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,
                          0.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f };

    m_VAO = std::make_unique<VertexArray>();
    unsigned int buffSize{ 5 * 6 * 6 * sizeof(float) };
    m_vertexBuffer = std::make_unique<VertexBuffer>(cubeVertices, buffSize);
    VertexBufferLayout layout;
    layout.push<float>(3);  // vertex position
    layout.push<float>(2);  // tex coords
    m_VAO->addBuffer(*m_vertexBuffer, layout);

    Renderer::setClearColor(0.1f, 0.3f, 0.4f, 1.0f);
}

TestCubes::~TestCubes() {}

void TestCubes::onUpdate(float deltaTime) {}

void TestCubes::onRender()
{
    m_view   = glm::translate(glm::mat4(1.0f), m_cameraPos);
    m_proj3D = glm::perspective(glm::radians(m_viewAngle), (float)settings::windowWidth / (float)settings::windowHeight,
                                0.1f, 100.0f);

    glm::mat4 projection;
    projection = m_proj2D;
    if (m_usePerspectiveProjection) {
        projection = m_proj3D;
    }

    // sending data to shader
    m_shader->setUniform1f("u_percent", m_visibilityRatio);
    m_shader->setUniformMat4f("u_view", 1, GL_FALSE, glm::value_ptr(m_view));
    m_shader->setUniformMat4f("u_projection", 1, GL_FALSE, glm::value_ptr(projection));

    for (int i = 0; i < 10; i++) {
        glm::mat4 model(1.0f);
        model = glm::translate(model, m_cubePositions[i]);
        float angle{ 20.0f * i - 30.0f };
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle), m_rotationAxis);
        m_shader->setUniformMat4f("u_model", 1, GL_FALSE, glm::value_ptr(model));
        // Renderer::draw(*m_VAO, *m_shader, 6 * 5, 6);
        Renderer::draw(*m_VAO, *m_shader);
    }
}

void TestCubes::onImGuiRender()
{
    ImGui::Checkbox("Use Perspective Projection", &m_usePerspectiveProjection);
    ImGui::Spacing();
    ImGui::SliderFloat("View Angle (FOV)", &m_viewAngle, 0.0f, 180.0f);
    ImGui::Spacing();
    ImGui::SliderFloat("Visibility Ratio", &m_visibilityRatio, 0.0f, 1.0f);
    ImGui::Spacing();
    ImGui::SliderFloat3("Camera Position", glm::value_ptr(m_cameraPos), -10.0f, 10.0f);
    ImGui::Spacing();
    ImGui::SliderFloat3("Axis of rotation", glm::value_ptr(m_rotationAxis), -1.0f, 1.0f);
    ImGui::Spacing();
}

}  // namespace tests
