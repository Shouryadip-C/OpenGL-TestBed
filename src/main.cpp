// Local headers
#include "Assets.h"
#include "Clock.h"
#include "Core.h"
#include "Glfw.h"
#include "GpuConfig.h"
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
#include "tests/TestModels.h"
#ifdef TESTBED_HAS_CUDA
    #include "tests/TestCuda.h"
#endif

// Third party headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

// Standard lib headers
#include <iostream>
#include <string>


#define internal      static
#define global_var    static
#define local_persist static


global_var bool              g_fillTriangles{ false };
global_var bool              g_showDemoWindow{ false };
global_var ScreenshotManager g_screenShotManager{ nullptr };
global_var Clock             g_clock;
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


internal void printGpuInfo()
{
    const GLubyte *renderer = glGetString(GL_RENDERER);  // GPU name
    const GLubyte *vendor   = glGetString(GL_VENDOR);    // NVIDIA / AMD / Intel
    const GLubyte *version  = glGetString(GL_VERSION);   // OpenGL version

    std::cout << "GPU Vendor:   " << vendor << "\n";
    std::cout << "GPU Renderer: " << renderer << "\n";
    std::cout << "OpenGL Ver:   " << version << "\n";
}


int main()
{
    // Set error callback before using glfw
    glfwSetErrorCallback([](int error, const char *description)
                         { std::cerr << "GLFW Error (" << error << ") - " << description << "\n"; });

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

    // Disable Vsync
    glfwSwapInterval(1);

    // Load GLAD opengl functions, gladLoadGLLoader() returns 0 if error occurs
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Print GPU info
    printGpuInfo();

    // get the maximum number of vertex attributes we can specify in vertex shader
    // glgetversion gets the opengl version that is loaded
    int nAttributes;
    GL_CALL(glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nAttributes));
    std::cout << "Max no. of vertex attributes is: " << nAttributes << std::endl;

    // Imgui setup ------------------------------------------------------------------------------ //
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    // keep the layout file next to the executable, ImGui stores the pointer so the string has to outlive it
    static const std::string imguiIniPath = (assets::executableDir() / "imgui.ini").string();
    io.IniFilename                        = imguiIniPath.c_str();
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
    g_testMenu->registerTest<tests::TestModels>("Model Loading");
#ifdef TESTBED_HAS_CUDA
    g_testMenu->registerTest<tests::TestCuda>("Cuda Simulation");
#endif

    // enable blending
    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // * renderer
    Renderer renderer;
    renderer.setClearColor(0.1f, 0.3f, 0.4f, 1.0f);

    // Our state
    float currFps{ g_clock.getFps() };
    float timeElapsed{ 400 };
    bool  limitFps{ true };
    bool  enableVSync{ true };
    // Setting fpsLimit to 0 -> unlimited FPS
    int fpsLimit{ 60 };

    // Clock to replace manual calculation
    g_clock.setTargetFps(fpsLimit);
    g_clock.start();

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window)) {
        // Measure whole frame time
        g_clock.beginFrame();
        timeElapsed += g_clock.getDeltaTime() * 1000;
        // Update Display of FPS text only after a certain time period so it is readable
        if (timeElapsed > 500.0f) {
            timeElapsed = 0;
            currFps     = g_clock.getFps();
        }

        // Process keyboard input --------------------------------------------------------------- //
        processInput(window);
        g_currentTest->processInput(window, g_clock.getDeltaTime() * 1000);

        // Render ------------------------------------------------------------------------------- //
        renderer.clear();

        // Render The active test --------------------------------------------------------------- //
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (g_currentTest) {
            g_currentTest->onUpdate(g_clock.getDeltaTime() * 1000);
            g_currentTest->onRender();

            ImGui::Begin("Test Menu", 0, settings::windowFlags);
            {
                if (ImGui::CollapsingHeader("Options", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Spacing();
                    g_currentTest->onImGuiRender();
                    ImGui::Spacing();

                    if (g_currentTest != g_testMenu) {
                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();
                        if (ImGui::Button("Back To Main Menu",
                                          ImVec2(-FLT_MIN, 1.2f * ImGui::GetTextLineHeightWithSpacing())))
                        {
                            delete g_currentTest;
                            g_currentTest = g_testMenu;
                        }
                        ImGui::Spacing();
                    }
                }

                if (ImGui::CollapsingHeader("Performance Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Spacing();

                    if (ImGui::Checkbox("Enable VSync", &enableVSync)) {
                        glfwSwapInterval(enableVSync ? 1 : 0);
                    }
                    ImGui::Spacing();

                    if (ImGui::Checkbox("Limit FPS", &limitFps)) {
                        g_clock.setTargetFps(limitFps ? fpsLimit : 0);
                    }

                    ImGui::Spacing();
                    ImGui::Spacing();

                    ImGui::Text("FPS:");
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0, 1, 0, 1), "%.2f", currFps);
                    ImGui::Spacing();
                    if (ImGui::SliderInt("Set FPS", &fpsLimit, 10, 300)) {
                        g_clock.setTargetFps(limitFps ? fpsLimit : 0);
                    }

                    ImGui::Spacing();
                }

                if (ImGui::CollapsingHeader("Debug Options", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Spacing();
                    ImGui::Checkbox("Show Demo Window", &g_showDemoWindow);
                    ImGui::Spacing();
                }
            }
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

        // Measure whole frame time
        g_clock.endFrame();
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
