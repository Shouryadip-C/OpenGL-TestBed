#include "TestCamera.h"

#include "Core.h"
#include "Glfw.h"
#include "Renderer.h"
#include "Settings.h"
#include "VertexBufferLayout.h"

// external
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

#include <cmath>
#include <iostream>


namespace tests {

TestCamera::TestCamera()
  : m_mouseCaptured(false),
    m_usePerspectiveProjection(true),
    m_visibilityRatio(0.0f),
    m_lastXPos(0.0f),
    m_lastYPos(0.0f),
    m_lastScroll(0.0f),
    m_cubePositions{ glm::vec3(0.0f, 0.0f, 0.0f),     glm::vec3(2.0f, 5.0f, -15.0f), glm::vec3(-1.5f, -2.2f, -2.5f),
                     glm::vec3(-3.8f, -2.0f, -12.3f), glm::vec3(2.4f, -0.4f, -3.5f), glm::vec3(-1.7f, 3.0f, -7.5f),
                     glm::vec3(1.3f, -2.0f, -2.5f),   glm::vec3(1.5f, 2.0f, -2.5f),  glm::vec3(1.5f, 0.2f, -1.5f),
                     glm::vec3(-1.3f, 1.0f, -1.5f) },
    m_proj2D(glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.0f, 20.0f)),
    m_proj3D(glm::perspective(glm::radians(settings::camera::zoom),
                              (float)settings::windowWidth / (float)settings::windowHeight,
                              0.1f,
                              100.0f)),
    m_rotationAxis(glm::vec3(0.5f, 1.0f, 0.0f)),
    m_camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
    m_view(glm::mat4(1.0f))
{
    // TODO: check if texture binding problems occur on copy construction of Texture object
    m_textures.push_back(std::make_unique<Texture>("../res/textures/awesomeface.png"));
    m_textures.push_back(std::make_unique<Texture>("../res/textures/hells_paradise.jpg"));

    for (unsigned int i = 0; i < m_textures.size(); i++) {
        m_textures[i]->bind(i);
    }

    m_shader = std::make_unique<Shader>("../res/shader/cube_shader.glsl");
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

TestCamera::~TestCamera() {}

void TestCamera::processMouseClick(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS && action != GLFW_REPEAT) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPos(window, m_lastXPos, m_lastYPos);
        m_mouseCaptured = true;
    }
}

void TestCamera::processMouseMovement(GLFWwindow *window, float xPos, float yPos)
{
    if (m_mouseCaptured) {
        m_camera.processMouseMove(xPos - m_lastXPos, m_lastYPos - yPos);
        glfwGetCursorPos(window, &m_lastXPos, &m_lastYPos);
    }
}

void TestCamera::processMouseScroll(GLFWwindow *window, float xPos, float yPos)
{
    if (m_mouseCaptured) {
        m_camera.processMouseScroll(yPos - m_lastScroll);
    }
}

void TestCamera::processInput(GLFWwindow *window, const float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_mouseCaptured = false;
    }

    Movement direction{ Movement::None };
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        direction = Movement::Front;
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        direction = Movement::Back;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        direction = Movement::Left;
    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        direction = Movement::Right;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        direction = Movement::Up;
    }
    else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        direction = Movement::Down;
    }
    m_camera.processKeyboardEvents(direction, deltaTime);
}

void TestCamera::onUpdate(float deltaTime) {}

void TestCamera::onRender()
{
    // * Revolve the camera in a circle
    // const float radius{ 10.0f };
    // float       camX{ sinf((float)glfwGetTime()) * radius };
    // float       camZ{ cosf((float)glfwGetTime()) * radius };
    // m_view   = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

    m_view   = m_camera.getViewMatrix();
    m_proj3D = glm::perspective(glm::radians(m_camera.zoom),
                                (float)settings::windowWidth / (float)settings::windowHeight, 0.1f, 100.0f);

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
        Renderer::draw(*m_VAO, *m_shader);
    }
}

void TestCamera::onImGuiRender()
{
    ImGui::Checkbox("Use Perspective Projection", &m_usePerspectiveProjection);
    ImGui::Spacing();
    ImGui::SliderFloat("Visibility Ratio", &m_visibilityRatio, 0.0f, 1.0f);
    ImGui::Spacing();
    ImGui::SliderFloat("Camera Speed", &m_camera.movementSpeed, 0.01f, 0.05f);
    ImGui::Spacing();
    ImGui::SliderFloat("Camera Zoom", &m_camera.zoom, 10.0f, 90.0f);
    ImGui::Spacing();
    ImGui::SliderFloat3("Axis of rotation", glm::value_ptr(m_rotationAxis), -1.0f, 1.0f);
    ImGui::Spacing();
}

}  // namespace tests
