#include "ScreenshotManager.h"

#include "Assets.h"
#include "Core.h"
#include <stb_image/stb_image_write.h>

#include <chrono>
#include <cstring>
#include <iomanip>
#include <sstream>


ScreenshotManager::ScreenshotManager(GLFWwindow *win) : m_window(win) {}

void ScreenshotManager::setWindow(GLFWwindow *win)
{
    m_window = win;
}

// Generate a timestamped filename
std::string ScreenshotManager::generateFilename()
{
    auto now  = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms   = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << "screenshot_" << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S_") << std::setfill('0') << std::setw(3)
       << ms.count() << ".png";

    return (assets::rootDir() / "screenshots" / ss.str()).string();
}

// Capture and save screenshot
bool ScreenshotManager::captureScreenshot()
{
    // Get window dimensions
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);

    // Allocate memory for pixel data (RGBA)
    std::vector<unsigned char> pixels(width * height * 4);

    // Read pixels from framebuffer
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // Flip the image vertically (OpenGL has origin at bottom-left)
    flipImageVertically(pixels, width, height);

    // Generate filename and save
    std::string filename = generateFilename();

    // Save as PNG
    int success = stbi_write_png(filename.c_str(), width, height, 4, pixels.data(), width * 4);

    if (success) {
        return true;
    }
    else {
        return false;
    }
}

// Flip image vertically (OpenGL reads from bottom-left)
void ScreenshotManager::flipImageVertically(std::vector<unsigned char> &pixels, int width, int height)
{
    int                        rowSize = width * 4;
    std::vector<unsigned char> tempRow(rowSize);

    for (int y = 0; y < height / 2; ++y) {
        int topRow    = y * rowSize;
        int bottomRow = (height - 1 - y) * rowSize;

        // Swap rows
        memcpy(tempRow.data(), &pixels[topRow], rowSize);
        memcpy(&pixels[topRow], &pixels[bottomRow], rowSize);
        memcpy(&pixels[bottomRow], tempRow.data(), rowSize);
    }
}
