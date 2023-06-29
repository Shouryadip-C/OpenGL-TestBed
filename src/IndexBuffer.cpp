#include "IndexBuffer.h"

#include "Core.h"


IndexBuffer::IndexBuffer(const void *data, unsigned int count) : m_count(count)
{
    ASSERT(sizeof(unsigned int) == sizeof(GLuint));

    GL_CALL(glGenBuffers(1, &m_rendererID));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID));
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}

IndexBuffer::~IndexBuffer()
{
    GL_CALL(glDeleteBuffers(1, &m_rendererID));
}

void IndexBuffer::bind() const
{
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID));
}

void IndexBuffer::unbind() const
{
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
