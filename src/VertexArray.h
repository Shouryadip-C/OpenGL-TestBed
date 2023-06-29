#pragma once

#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

class VertexArray
{
private:
    unsigned int m_rendererID;
    unsigned int m_indexCount;
    unsigned int m_vertexCount;

public:
    VertexArray();
    ~VertexArray();

    void addBuffer(const VertexBuffer &vb, const VertexBufferLayout &layout);
    void addBuffer(const IndexBuffer &ib);

    void bind() const;
    void unbind() const;

    inline unsigned int getIndexCount() const { return m_indexCount; };
    inline unsigned int getVertexCount() const { return m_vertexCount; };
};
