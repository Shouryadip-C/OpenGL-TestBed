#include "Texture.h"

#include "Core.h"
#include <stb_image/stb_image.h>

#include <iostream>

unsigned int loadTextureFromFile(const std::filesystem::path &path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int            width, height, nrComponents;
    unsigned char *data = stbi_load(path.string().c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        // stb_image returns 1 (grey), 2 (grey + alpha), 3 (rgb) or 4 (rgba) components
        GLenum format = GL_RGBA;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 2)
            format = GL_RG;
        else if (nrComponents == 3)
            format = GL_RGB;

        // rows of 1, 2 and 3 component images are not always a multiple of the default 4 byte alignment
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

Texture::Texture(const std::filesystem::path &path)
  : m_rendererID(0), m_filePath(path), m_width(0), m_height(0), m_BPP(0)
{
    stbi_set_flip_vertically_on_load(1);
    // TODO: Fix runtime errors when building on windows with mingw clang compiler
    unsigned char *data = stbi_load(path.string().c_str(), &m_width, &m_height, &m_BPP, 4);

    GL_CALL(glGenTextures(1, &m_rendererID));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, m_rendererID));

    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

    if (data) {
        GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
        GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
    }
    else {
        std::cerr << "Failed to load texture"
                  << "\n";
    }
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

    if (data) {
        stbi_image_free(data);
    }
}

Texture::~Texture()
{
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    GL_CALL(glDeleteTextures(1, &m_rendererID));
    GL_CALL(glActiveTexture(GL_TEXTURE0));
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
