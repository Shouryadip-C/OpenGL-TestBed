#include "Core.h"

#include <iostream>


void clearGLErrors()
{
    while (glGetError() != GL_NONE) {
    }
}

bool logGLErrors(const char *function, const char *file, int line)
{
    while (unsigned int error = glGetError()) {
        std::cerr << "[OpenGL Error] (" << error << ") in " << function << "\n" << file << " : " << line << "\n";
        return false;
    }
    return true;
}
