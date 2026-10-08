#pragma once

#include "Core.h"

#include <vector>

struct VertexBufferElement
{
    unsigned int  type;
    unsigned int  count;
    unsigned char normalized;

    static unsigned int getSizeOfType(unsigned int type)
    {
        switch (type) {
            case GL_FLOAT: return 4;
            case GL_INT: return 4;
            case GL_UNSIGNED_INT: return 4;
            case GL_UNSIGNED_BYTE: return 1;
        }
        ASSERT(false);
        return 0;
    }
};


class VertexBufferLayout
{
private:
    std::vector<VertexBufferElement> m_elements;
    unsigned int                     m_stride;

public:
    VertexBufferLayout() : m_stride(0) {}
    ~VertexBufferLayout() {}

    // Only the specializations below are defined, using any other type is a compile error
    template<typename T>
    void push(unsigned int count) = delete;

    inline const std::vector<VertexBufferElement> &getElements() const { return m_elements; };
    inline unsigned int                            getStride() const { return m_stride; };
};


// Explicit specializations have to live at namespace scope, GCC rejects them inside the class body
template<>
inline void VertexBufferLayout::push<float>(unsigned int count)
{
    m_elements.push_back({ GL_FLOAT, count, GL_FALSE });
    m_stride += count * VertexBufferElement::getSizeOfType(GL_FLOAT);
}

template<>
inline void VertexBufferLayout::push<int>(unsigned int count)
{
    m_elements.push_back({ GL_INT, count, GL_FALSE });
    m_stride += count * VertexBufferElement::getSizeOfType(GL_INT);
}

template<>
inline void VertexBufferLayout::push<unsigned int>(unsigned int count)
{
    m_elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
    m_stride += count * VertexBufferElement::getSizeOfType(GL_UNSIGNED_INT);
}

template<>
inline void VertexBufferLayout::push<unsigned char>(unsigned int count)
{
    m_elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
    m_stride += count * VertexBufferElement::getSizeOfType(GL_UNSIGNED_BYTE);
}
