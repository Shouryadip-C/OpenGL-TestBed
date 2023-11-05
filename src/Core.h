#pragma once

#include "DebugTrap.h"
#include <glad/glad.h>


#define DEBUG_BREAK() psnip_trap()

#define ASSERT(x) \
    if (!(x)) DEBUG_BREAK();

#if defined(_DEBUG) || defined(DEBUG)
    #define GL_CALL(x)   \
        clearGLErrors(); \
        x;               \
        ASSERT(logGLErrors(#x, __FILE__, __LINE__))
#else
    #define GL_CALL(x) x
#endif

void clearGLErrors();
bool logGLErrors(const char *function, const char *file, int line);
