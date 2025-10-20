#pragma once

// GPU selection for hybrid systems (Intel + NVIDIA)
// Works on MSVC and Clang (Windows/Linux via WSL with NVIDIA passthrough)

#ifdef _WIN32
extern "C"
{
    // Forces usage of NVIDIA GPU on systems with both Intel + NVIDIA GPUs
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;

    // Forces usage of AMD discrete GPU (does nothing if no AMD GPU)
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#elif defined(__clang__) || defined(__GNUC__)
extern "C"
{
    // On Linux, this usually isn’t required because GLX/Vulkan drivers choose automatically,
    // but exporting these symbols doesn't hurt if running via NVIDIA Optimus.
    __attribute__((visibility("default"))) unsigned long NvOptimusEnablement                  = 0x00000001;
    __attribute__((visibility("default"))) int           AmdPowerXpressRequestHighPerformance = 1;
}
#endif
