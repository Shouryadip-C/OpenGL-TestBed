// Local headers
#include "Core.h"
#include "Glfw.h"
#include "Settings.h"

// Third party headers
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

// Standard lib headers
#include <iostream>


#define internal      static
#define global_var    static
#define local_persist static


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

    // Our state
    bool showDemoWindow{ true };

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window)) {
        // Process keyboard input --------------------------------------------------------------- //
        processInput(window);

        GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        // Render imgui window ------------------------------------------------------------------ //
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

        // Swap front and back buffers ---------------------------------------------------------- //
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
