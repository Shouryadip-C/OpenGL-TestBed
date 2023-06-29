#pragma once

class VertexBuffer
{
private:
    unsigned int m_rendererID;
    unsigned int m_bufferSize;

public:
    VertexBuffer(const void *data, unsigned int size);
    ~VertexBuffer();

    void bind() const;
    void unbind() const;

    inline unsigned int getBufferSize() const { return m_bufferSize; };
};
