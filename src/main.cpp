// Local headers
#include "Core.h"
#include "Glfw.h"
#include "IndexBuffer.h"
#include "Renderer.h"
#include "ScreenshotManager.h"
#include "Settings.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "tests/Test.h"
#include "tests/Test2DTransforms.h"
#include "tests/TestCamera.h"
#include "tests/TestClearColor.h"
#include "tests/TestCubes.h"

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


global_var bool              g_fillTriangles{ false };
global_var bool              g_showDemoWindow{ false };
global_var ScreenshotManager g_screenShotManager{ nullptr };
global_var tests::Test *g_currentTest{ nullptr };
global_var tests::TestMenu *g_testMenu{ nullptr };


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


internal void cursorPosCallback(GLFWwindow *window, double xPos, double yPos)
{
    if (g_currentTest) {
        g_currentTest->processMouseMovement(window, xPos, yPos);
    }
}


internal void mouseScrollCallback(GLFWwindow *window, double xPos, double yPos)
{
    if (g_currentTest) {
        g_currentTest->processMouseScroll(window, xPos, yPos);
    }
}


internal void mouseClickCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (g_currentTest) {
        g_currentTest->processMouseClick(window, button, action, mods);
    }
}


internal void processInput(GLFWwindow *window)
{
    static bool wasF12Pressed   = false;
    static bool wasSpacePressed = false;
    bool        isF12Down       = glfwGetKey(window, GLFW_KEY_F12) == GLFW_PRESS;
    bool        isSpaceDown     = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        g_showDemoWindow = true;
    }
    else if (isF12Down && !wasF12Pressed) {
        bool result = g_screenShotManager.captureScreenshot();
        if (result) {
            // TODO: Show notification of saved screenshot
        }
        else {
            // TODO: Show notification containing error message
        }
    }
    else if (isSpaceDown && !wasSpacePressed) {
        if (!g_fillTriangles) {
            GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
            g_fillTriangles = true;
        }
        else {
            GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
            g_fillTriangles = false;
        }
    }

    // Update current state of keys
    wasF12Pressed   = isF12Down;
    wasSpacePressed = isSpaceDown;
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

    // Configure which window to take screenshot from
    g_screenShotManager.setWindow(window);

    // set the resize window function callback
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);
    glfwSetMouseButtonCallback(window, mouseClickCallback);

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

    // Setup the test menu and tests
    g_testMenu    = new tests::TestMenu(g_currentTest);
    g_currentTest = g_testMenu;

    g_testMenu->registerTest<tests::TestClearColor>("Clear Color");
    g_testMenu->registerTest<tests::Test2DTransforms>("2D Transformations");
    g_testMenu->registerTest<tests::TestCubes>("3D Rotating Cubes");
    g_testMenu->registerTest<tests::TestCamera>("Movable 3D Camera");

    // enable blending
    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // * renderer
    Renderer renderer;
    renderer.setClearColor(0.1f, 0.3f, 0.4f, 1.0f);

    // Our state
    bool  setCallbacks{ true };
    float currentFrameTime{};
    float lastFrameTime{};
    float dt{};

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window)) {
        // Calculate deltatime for framerate independent movement
        currentFrameTime = glfwGetTime();
        dt               = (currentFrameTime - lastFrameTime) * 1000;
        lastFrameTime    = currentFrameTime;

        // Process keyboard input --------------------------------------------------------------- //
        processInput(window);
        g_currentTest->processInput(window, dt);

        // Render ------------------------------------------------------------------------------- //
        renderer.clear();

        // Render The active test --------------------------------------------------------------- //
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (g_currentTest) {
            g_currentTest->onUpdate(dt);
            g_currentTest->onRender();

            ImGui::Begin("Test Menu", 0, settings::windowFlags);
            g_currentTest->onImGuiRender();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            if (g_currentTest != g_testMenu
                && ImGui::Button("Back To Main Menu", ImVec2(-FLT_MIN, 1.2 * ImGui::GetTextLineHeightWithSpacing())))
            {
                delete g_currentTest;
                g_currentTest = g_testMenu;
            }
            ImGui::Checkbox("Show Demo Window", &g_showDemoWindow);
            ImGui::End();
        }

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

        // Swap front and back buffers ---------------------------------------------------------- //
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // TODO: Deallocate all objects after use --------------------------------------------------- //
    delete g_currentTest;
    if (g_currentTest != g_testMenu) {
        delete g_testMenu;
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
