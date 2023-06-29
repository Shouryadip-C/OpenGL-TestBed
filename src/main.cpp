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
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

// Standard lib headers
#include <iostream>


#define internal      static
#define global_var    static
#define local_persist static


global_var bool  g_fillTriangles{ false };
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
            0.0f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 1.0f,  // top middle
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

        Renderer renderer;
        renderer.setClearColor(0.1f, 0.3f, 0.4f, 1.0f);

        // unbind the currently bound VBO and VAO
        // NOTE: Unbind the VAO before any other buffers as VAO stores unbind calls too
        // GL_CALL(glBindVertexArray(0));
        // GL_CALL(glUseProgram(0));
        // GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        // GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        // Our state
        bool showDemoWindow{ true };

        // Loop until the user closes the window
        while (!glfwWindowShouldClose(window)) {
            // Process keyboard input ----------------------------------------------------------- //
            processInput(window);

            // Render --------------------------------------------------------------------------- //
            renderer.clear();

            // set the color uniform
            basicShader.bind();

            // NOTE: bind the shader program to use before calling this or glUniform4f
            // might throw error: 'ERROR 1282 in glUniform4f'
            basicShader.setUniform1f("u_percent", g_visibilityRatio);

            // render the triangles
            renderer.draw(va, basicShader, 6, 3);

            // Render imgui window -------------------------------------------------------------- //
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if (showDemoWindow) {
                ImGui::ShowDemoWindow(&showDemoWindow);
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
