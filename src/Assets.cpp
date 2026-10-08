#include "Assets.h"

#include <system_error>

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
#endif


namespace assets {

static std::filesystem::path findExecutableDir()
{
#if defined(_WIN32)
    wchar_t buffer[MAX_PATH];
    DWORD   length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (length > 0 && length < MAX_PATH) {
        return std::filesystem::path(buffer).parent_path();
    }
#elif defined(__linux__)
    std::error_code       error;
    std::filesystem::path exe = std::filesystem::read_symlink("/proc/self/exe", error);
    if (!error) {
        return exe.parent_path();
    }
#endif
    // Unknown platform or the lookup failed, behave like before and rely on the working directory
    return std::filesystem::current_path();
}

const std::filesystem::path &executableDir()
{
    static const std::filesystem::path dir = findExecutableDir();
    return dir;
}

const std::filesystem::path &rootDir()
{
    static const std::filesystem::path dir = executableDir().parent_path();
    return dir;
}

std::filesystem::path path(const std::filesystem::path &relativePath)
{
    return rootDir() / "res" / relativePath;
}

}  // namespace assets
