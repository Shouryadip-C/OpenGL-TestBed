#include "Test2DTransforms.h"

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

Test2DTransforms::Test2DTransforms()
  : m_triangleNumber(0),
    m_usePerspectiveProjection(false),
    m_viewAngle(60.0f),
    m_viewBoundaries{ -2.0f, 2.0f, -2.0f, 2.0f },
    m_visibilityRatio(0.0f),
    m_proj3D(glm::perspective(glm::radians(m_viewAngle),
                              (float)settings::windowWidth / (float)settings::windowHeight,
                              0.1f,
                              100.0f)),
    m_rotationAxis(glm::vec3(0.0f, 0.0f, 1.0f))
{
    // creating textures
    m_textures.push_back(std::make_unique<Texture>("../res/textures/awesomeface.png"));
    m_textures.push_back(std::make_unique<Texture>("../res/textures/hells_paradise.jpg"));

    for (unsigned int i = 0; i < m_textures.size(); i++) {
        m_textures[i]->bind(i);
    }

    // Creating the shader program
    m_shader = std::make_unique<Shader>("../res/shader/basic_shader.glsl");
    m_shader->bind();
    m_shader->setUniform1i("u_texture1", 0);
    m_shader->setUniform1i("u_texture2", 1);

    // clang-format off
    // Vertex and index buffers and vertex data
    float vertices[]{
        // x    y     z    |     colors      | tex coords
        0.0f,  0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.5f, 1.0f,  // top middle
        0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom left
        -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // top left
        0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top right
        // Hexagon
        -0.5f,  0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.75f,  // left top
        -0.5f, -0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.25f,  // left bottom
        0.5f,   0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.75f,  // right top
        0.5f,  -0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.25f,  // right bottom
        0.0f,   -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.0f,  // bottom middle
    };

    unsigned int indices[]{
        0, 1, 2,  // first triangle
        1, 2, 3,  // second triangle
        3, 4, 1,   // third triangle
        // Hexagon
        0, 5, 6,
        6, 9, 0,
        0, 7, 8,
        8, 9, 0
    };
    // clang-format on

    m_VAO = std::make_unique<VertexArray>();
    unsigned int buffSize{ 8 * 10 * sizeof(float) };
    m_vertexBuffer = std::make_unique<VertexBuffer>(vertices, buffSize);
    m_indexBuffer  = std::make_unique<IndexBuffer>(indices, 21);

    VertexBufferLayout layout;
    layout.push<float>(3);  // vertex position
    layout.push<float>(3);  // vertex color
    layout.push<float>(2);  // tex coords

    m_VAO->addBuffer(*m_vertexBuffer, layout);
    m_VAO->addBuffer(*m_indexBuffer);

    Renderer::setClearColor(0.1f, 0.3f, 0.4f, 1.0f);
}

Test2DTransforms::~Test2DTransforms() {}

void Test2DTransforms::onUpdate(float deltaTime) {}

void Test2DTransforms::onRender()
{
    m_proj3D = glm::perspective(glm::radians(m_viewAngle), (float)settings::windowWidth / (float)settings::windowHeight,
                                0.1f, 100.0f);

    glm::mat4 projection;
    projection =
        glm::ortho(m_viewBoundaries[0], m_viewBoundaries[1], m_viewBoundaries[2], m_viewBoundaries[3], -10.0f, 100.0f);
    if (m_usePerspectiveProjection) {
        projection = m_proj3D;
    }

    // sending data to shader
    m_shader->setUniform1f("u_percent", m_visibilityRatio);
    m_shader->setUniformMat4f("u_projection", 1, GL_FALSE, glm::value_ptr(projection));

    // transform first image
    m_shader->setUniform1f("u_percent", m_visibilityRatio);
    glm::mat4 transform{ glm::mat4(1.0f) };
    float     scale{ sinf((float)glfwGetTime()) };
    transform = glm::scale(transform, glm::vec3(scale, scale, 1));
    transform = glm::translate(transform, glm::vec3(-0.5f, 0.5f, -5.0f));
    transform = glm::rotate(transform, (float)glfwGetTime(), m_rotationAxis);
    m_shader->setUniformMat4f("u_transform", 1, GL_FALSE, glm::value_ptr(transform));
    Renderer::draw(*m_VAO, *m_shader, 3, 3 * m_triangleNumber);

    // transform second image
    m_shader->setUniform1f("u_percent", 1 - m_visibilityRatio);
    transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, -5.0f));
    transform = glm::rotate(transform, (float)glfwGetTime(), m_rotationAxis);
    transform = glm::translate(transform, glm::vec3(0.5f, -0.5f, 0.0f));
    m_shader->setUniformMat4f("u_transform", 1, GL_FALSE, glm::value_ptr(transform));
    Renderer::draw(*m_VAO, *m_shader, 6, 3);

    // Render Hexagon
    transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(0.0f, -1.5f, 0.0f));
    m_shader->setUniformMat4f("u_transform", 1, GL_FALSE, glm::value_ptr(transform));
    m_shader->setUniform1f("u_percent", 0.0f);
    Renderer::draw(*m_VAO, *m_shader, 12, 9);
}

void Test2DTransforms::onImGuiRender()
{
    ImGui::Checkbox("Use Perspective Projection", &m_usePerspectiveProjection);
    ImGui::Spacing();
    ImGui::SliderInt("Triangle No.", &m_triangleNumber, 0, 2);
    ImGui::Spacing();
    ImGui::SliderFloat("View Angle (FOV)", &m_viewAngle, 0.0f, 180.0f);
    ImGui::Spacing();
    ImGui::SliderFloat("Visibility Ratio", &m_visibilityRatio, 0.0f, 1.0f);
    ImGui::Spacing();
    ImGui::SliderFloat3("Axis of rotation", glm::value_ptr(m_rotationAxis), -1.0f, 1.0f);
    ImGui::Spacing();
    ImGui::SliderFloat4("View Bounds", m_viewBoundaries, -10.0f, 10.0f);
    ImGui::Spacing();
}

}  // namespace tests
