#pragma once

#include <filesystem>

unsigned int loadTextureFromFile(const std::filesystem::path &path);

class Texture
{
private:
    unsigned int          m_rendererID;
    std::filesystem::path m_filePath;
    int                   m_width;
    int                   m_height;
    int                   m_BPP;

public:
    Texture(const std::filesystem::path &path);
    ~Texture();

    void bind(unsigned int slot = 0) const;
    void unbind() const;

    inline int                          getWidth() const { return m_width; }
    inline int                          getHeight() const { return m_height; }
    inline const std::filesystem::path &getPath() const { return m_filePath; };
};
