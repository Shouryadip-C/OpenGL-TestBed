#include "Glfw.h"

#include <string>
#include <vector>


class ScreenshotManager
{
private:
    GLFWwindow *m_window;

public:
    ScreenshotManager(GLFWwindow *win);

    void setWindow(GLFWwindow *win);

    // Generate a timestamped filename
    std::string generateFilename();

    // Capture and save screenshot
    bool captureScreenshot();

private:
    // Flip image vertically (OpenGL reads from bottom-left)
    void flipImageVertically(std::vector<unsigned char> &pixels, int width, int height);
};
