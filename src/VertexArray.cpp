#include "VertexArray.h"

#include "Core.h"

#include <cstdint>

VertexArray::VertexArray() : m_indexCount(0)
{
    GL_CALL(glGenVertexArrays(1, &m_rendererID));
}

VertexArray::~VertexArray()
{
    GL_CALL(glDeleteVertexArrays(1, &m_rendererID));
}

void VertexArray::addBuffer(const VertexBuffer &vb, const VertexBufferLayout &layout)
{
    bind();
    vb.bind();

    const auto  &elements{ layout.getElements() };
    unsigned int offset{ 0 };
    for (int i = 0; i < elements.size(); i++) {
        const auto &element{ elements[i] };
        GL_CALL(glVertexAttribPointer(i,
                                      element.count,
                                      element.type,
                                      element.normalized,
                                      layout.getStride(),
                                      (void *)(uintptr_t)offset));
        GL_CALL(glEnableVertexAttribArray(i));
        offset += element.count * VertexBufferElement::getSizeOfType(element.type);
    }

    unbind();
}

void VertexArray::addBuffer(const IndexBuffer &ib)
{
    m_indexCount = ib.getCount();
    bind();
    ib.bind();
    unbind();
}

void VertexArray::bind() const
{
    GL_CALL(glBindVertexArray(m_rendererID));
}

void VertexArray::unbind() const
{
    GL_CALL(glBindVertexArray(0));
}
