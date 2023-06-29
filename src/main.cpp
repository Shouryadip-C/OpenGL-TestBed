// Local headers
#include "Core.h"
#include "Glfw.h"
#include "IndexBuffer.h"
#include "Renderer.h"
#include "Settings.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

// Third party headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

// Standard lib headers
#include <iostream>


#define internal      static
#define global_var    static
#define local_persist static


global_var bool  g_fillTriangles{ false };
global_var bool  g_render3DExamples{ false };
global_var bool  g_showDemoWindow{ false };
global_var float g_visibilityRatio{ 0.0f };


// function opengl calls every time an error occurs
internal void APIENTRY errorCallback(unsigned int source,
                                     unsigned int type,
                                     unsigned int id,
                                     unsigned int severity,
                                     int          length,
                                     const char  *message,
                                     const void  *userParam)
{
    std::cerr << severity << ": " << id << "\n" << message << "\n";
    DEBUG_BREAK();
}


// callback function for every time the window is resized
internal void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    // set the opengl render area
    GL_CALL(glViewport(0, 0, width, height));
}


internal void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        g_showDemoWindow = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        g_render3DExamples = false;
    }
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        g_render3DExamples = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_REPEAT) {
        if (!g_fillTriangles) {
            GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
            g_fillTriangles = true;
        }
        else {
            GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
            g_fillTriangles = false;
        }
    }
    else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        g_visibilityRatio = std::min(1.0f, g_visibilityRatio + 0.01f);
    }
    else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        g_visibilityRatio = std::max(0.0f, g_visibilityRatio - 0.01f);
    }
}


int main()
{
    // Initialize the library
    if (!glfwInit()) {
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

#ifdef __APPLE__
    // reqired for macosx devices
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a windowed mode window and its OpenGL context
    GLFWwindow *window;
    window = glfwCreateWindow(settings::windowWidth, settings::windowHeight, "OpenGL TestBed", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // set the resize window function callback
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    // centering the window
    GLFWmonitor       *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode    = glfwGetVideoMode(monitor);
    glfwSetWindowPos(window, mode->width - settings::windowWidth - 10, (mode->height - settings::windowHeight) / 2);

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Load GLAD opengl functions, gladLoadGLLoader() returns 0 if error occurs
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // get the maximum number of vertex attributes we can specify in vertex shader
    // glgetversion gets the opengl version that is loaded
    int nAttributes;
    GL_CALL(glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nAttributes));
    std::cout << glGetString(GL_VERSION) << "\n"
              << "Max no. of vertex attributes is: " << nAttributes << std::endl;

    // Imgui setup ------------------------------------------------------------------------------ //
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding              = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430 core");
    // Imgui setup END -------------------------------------------------------------------------- //


    // Create a new scope to avoid opengl errors due to glfwterminate() deleting the opengl
    // context before destruction of resources that call the opengl functions
    {

        // enable blending
        GL_CALL(glEnable(GL_BLEND));
        GL_CALL(glEnable(GL_DEPTH_TEST));
        GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        // Creating the shader program
        Shader basicShader("../res/shader/basic_shader.glsl");

        // loading the image texture
        Texture image1("../res/textures/awesomeface.png");
        Texture image2("../res/textures/hells_paradise.jpg");
        image1.bind();
        image2.bind(1);
        basicShader.bind();
        basicShader.setUniform1i("u_texture1", 0);
        basicShader.setUniform1i("u_texture2", 1);

        // Vertex and index buffers and vertex data
        float vertices[]{
            // x    y     z    |     colors      | tex coords
            0.0f,  0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.5f, 1.0f,  // top middle
            0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom left
            -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // top left
            0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top right
        };

        unsigned int indices[]{
            0, 1, 2,  // first triangle
            1, 2, 3,  // second triangle
            3, 4, 1   // third triangle
        };

        VertexArray va;

        VertexBuffer       vb(&vertices, 8 * 5 * sizeof(float));
        VertexBufferLayout layout;
        layout.push<float>(3);
        layout.push<float>(3);
        layout.push<float>(2);

        IndexBuffer ib(indices, 9);

        va.addBuffer(vb, layout);
        va.addBuffer(ib);

        // 3D Cube stuff ------------------------------------------------------------------------ //
        Shader cubeShader("../res/shader/cube_shader.glsl");
        cubeShader.bind();
        cubeShader.setUniform1i("u_texture1", 0);
        cubeShader.setUniform1i("u_texture2", 1);

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

        glm::vec3 cubePositions[]{ glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
                                   glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
                                   glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
                                   glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
                                   glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f) };

        VertexArray        vaCube;
        VertexBuffer       vbCube(&cubeVertices, 5 * 6 * 6 * sizeof(float));
        VertexBufferLayout cubeVbLayout;
        cubeVbLayout.push<float>(3);
        cubeVbLayout.push<float>(2);
        vaCube.addBuffer(vbCube, cubeVbLayout);


        // * renderer
        Renderer renderer;
        renderer.setClearColor(0.1f, 0.3f, 0.4f, 1.0f);


        // Math stuff --------------------------------------------------------------------------- //
        // glm::mat4 trans = glm::mat4(1.0f);
        // trans           = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
        // trans           = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));

        // * 3D rendering
        glm::mat4 view{ glm::mat4(1.0f) };
        // note that we're translating the scene in the reverse direction of where we want to move
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

        glm::mat4 proj3D;
        proj3D = glm::perspective(glm::radians(60.0f), (float)settings::windowWidth / (float)settings::windowHeight,
                                  0.1f, 100.0f);
        // proj3D = glm::ortho(-9.0f, 9.0f, -6.0f, 6.0f, 0.0f, 50.0f);

        glm::mat4 proj2D{ glm::mat4(1.0f) };
        proj2D = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -1.0f, 1.0f);


        // Loop until the user closes the window
        while (!glfwWindowShouldClose(window)) {
            // Process keyboard input ----------------------------------------------------------- //
            processInput(window);

            // Render --------------------------------------------------------------------------- //
            renderer.clear();

            if (!g_render3DExamples) {
                // set the color uniform
                basicShader.bind();
                // float timeValue{ static_cast<float>(glfwGetTime()) };
                // float greenValue{ (std::sin(timeValue) / 2.0f) + 0.5f };
                // NOTE: bind the shader program to use before calling this or glUniform4f
                // might throw error: 'ERROR 1282 in glUniform4f'
                // basicShader.setUniform4f("u_color", 0.0f, greenValue, 0.0f, 1.0f);

                basicShader.setUniformMat4f("u_projection", 1, GL_FALSE, glm::value_ptr(proj2D));

                // transform the image
                basicShader.setUniform1f("u_percent", g_visibilityRatio);
                glm::mat4 trans2{ glm::mat4(1.0f) };
                float     scale{ std::sinf((float)glfwGetTime()) };
                trans2 = glm::scale(trans2, glm::vec3(scale, scale, 1));
                trans2 = glm::translate(trans2, glm::vec3(-0.5f, 0.5f, 0.0f));
                trans2 = glm::rotate(trans2, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
                basicShader.setUniformMat4f("u_transform", 1, GL_FALSE, glm::value_ptr(trans2));
                renderer.draw(va, basicShader, 3, 0);

                // transform the image
                basicShader.setUniform1f("u_percent", 1 - g_visibilityRatio);
                glm::mat4 trans{ glm::mat4(1.0f) };
                trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
                trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));
                basicShader.setUniformMat4f("u_transform", 1, GL_FALSE, glm::value_ptr(trans));
                renderer.draw(va, basicShader, 6, 3);
            }
            else {
                // sending data to shader
                cubeShader.bind();
                cubeShader.setUniform1f("u_percent", g_visibilityRatio);
                cubeShader.setUniformMat4f("u_view", 1, GL_FALSE, glm::value_ptr(view));
                cubeShader.setUniformMat4f("u_projection", 1, GL_FALSE, glm::value_ptr(proj3D));

                for (int i = 0; i < 10; i++) {
                    glm::mat4 model{ glm::mat4(1.0f) };
                    model = glm::translate(model, cubePositions[i]);
                    float angle{ 20.0f * i - 30.0f };
                    model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle), glm::vec3(0.5f, 1.0f, 0.0f));
                    cubeShader.setUniformMat4f("u_model", 1, GL_FALSE, glm::value_ptr(model));
                    // renderer.draw(vaCube, cubeShader, 6 * 5, 6);
                    renderer.draw(vaCube, cubeShader);
                }
            }


            // Render imgui window -------------------------------------------------------------- //
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if (g_showDemoWindow) {
                ImGui::ShowDemoWindow(&g_showDemoWindow);
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // enabling imgui viewports
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                GLFWwindow *backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context);
            }

            // Swap front and back buffers ------------------------------------------------------ //
            glfwSwapBuffers(window);

            // Poll for and process events
            glfwPollEvents();
        }

        // TODO: Deallocate all objects after use ----------------------------------------------- //
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
