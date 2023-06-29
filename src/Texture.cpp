#include "Texture.h"

#include "Core.h"
#include <stb_image/stb_image.h>

#include <iostream>


Texture::Texture(const std::filesystem::path &path)
  : m_rendererID(0), m_filePath(path), m_localBuffer(nullptr), m_width(0), m_height(0), m_BPP(0)
{
    stbi_set_flip_vertically_on_load(1);
    m_localBuffer = stbi_load(path.string().c_str(), &m_width, &m_height, &m_BPP, 4);

    GL_CALL(glGenTextures(1, &m_rendererID));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, m_rendererID));

    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

    if (m_localBuffer) {
        GL_CALL(
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_localBuffer));
        GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
    }
    else {
        std::cerr << "Failed to load texture"
                  << "\n";
    }
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

    if (m_localBuffer) {
        stbi_image_free(m_localBuffer);
    }
}

Texture::~Texture()
{
    GL_CALL(glDeleteTextures(1, &m_rendererID));
}

void Texture::bind(unsigned int slot) const
{
    GL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, m_rendererID));
}

void Texture::unbind() const
{
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}
