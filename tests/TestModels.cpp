#include "TestModels.h"

#include "Core.h"
#include "Glfw.h"
#include "Renderer.h"
#include "Settings.h"
#include "VertexBufferLayout.h"

// external
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

#include <cmath>
#include <iostream>


namespace tests {

TestModels::TestModels()
  : m_mouseCaptured(false),
    m_usePerspectiveProjection(true),
    m_lastXPos(0.0f),
    m_lastYPos(0.0f),
    m_lastScroll(0.0f),
    m_objFilePath("../res/models/backpack/backpack.obj"),
    m_proj2D(glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.0f, 20.0f)),
    m_proj3D(glm::perspective(glm::radians(settings::camera::zoom),
                              (float)settings::windowWidth / (float)settings::windowHeight,
                              0.1f,
                              100.0f)),
    m_camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
    m_view(glm::mat4(1.0f))
{
    m_shader = std::make_unique<Shader>("../res/shader/model_shader.glsl");

    m_model = std::make_unique<Model>(m_objFilePath);

    Renderer::setClearColor(0.1f, 0.3f, 0.4f, 1.0f);
}

TestModels::~TestModels() {}

void TestModels::processMouseClick(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS && action != GLFW_REPEAT) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPos(window, m_lastXPos, m_lastYPos);
        m_mouseCaptured = true;
    }
}

void TestModels::processMouseMovement(GLFWwindow *window, float xPos, float yPos)
{
    if (m_mouseCaptured) {
        m_camera.processMouseMove(xPos - m_lastXPos, m_lastYPos - yPos);
        glfwGetCursorPos(window, &m_lastXPos, &m_lastYPos);
    }
}

void TestModels::processMouseScroll(GLFWwindow *window, float xPos, float yPos)
{
    if (m_mouseCaptured) {
        m_camera.processMouseScroll(yPos - m_lastScroll);
    }
}

void TestModels::processInput(GLFWwindow *window, const float deltaTime)
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

void TestModels::onUpdate(float deltaTime) {}

void TestModels::onRender()
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
    m_shader->setUniformMat4f("view", 1, GL_FALSE, glm::value_ptr(m_view));
    m_shader->setUniformMat4f("projection", 1, GL_FALSE, glm::value_ptr(projection));

    glm::mat4 model(1.0f);
    m_shader->setUniformMat4f("model", 1, GL_FALSE, glm::value_ptr(model));

    m_model->draw(*m_shader);
}

void TestModels::onImGuiRender()
{
    ImGui::Checkbox("Use Perspective Projection", &m_usePerspectiveProjection);
    ImGui::Spacing();
    ImGui::SliderFloat("Camera Speed", &m_camera.movementSpeed, 0.01f, 0.05f);
    ImGui::Spacing();
    ImGui::SliderFloat("Camera Zoom", &m_camera.zoom, 10.0f, 90.0f);
    ImGui::Spacing();

    // Render a button to open the file dialog
    if (ImGui::Button("Choose Obj File")) {
        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",  // dialog key
                                                "Choose a File",     // title
                                                ".*",                // filter (accept all)
                                                { "../" }            // starting directory
        );
    }
    ImGui::Spacing();

    // Display file dialog when it's open
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            m_objFilePath = ImGuiFileDialog::Instance()->GetFilePathName();
        }

        m_model = std::make_unique<Model>(m_objFilePath);

        // must be called to close
        ImGuiFileDialog::Instance()->Close();
    }

    // Show selected file path
    if (!m_objFilePath.empty()) {
        ImGui::Text("Selected File:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", m_objFilePath.filename().c_str());
    }
    ImGui::Spacing();
}

}  // namespace tests
