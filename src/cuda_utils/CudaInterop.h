#pragma once

// Include your core OpenGL loader first
#include <glad/glad.h>

// Then CUDA headers
#include <cuda_gl_interop.h>
#include <cuda_runtime.h>

#include <iostream>


#define CUDA_CHECK(x)                                                                          \
    do {                                                                                       \
        cudaError_t err = x;                                                                   \
        if (err != cudaSuccess) {                                                              \
            std::cerr << "CUDA Error " << #x << " at " << __FILE__ << ":" << __LINE__ << " — " \
                      << cudaGetErrorString(err) << "\n";                                      \
        }                                                                                      \
    } while (0)
