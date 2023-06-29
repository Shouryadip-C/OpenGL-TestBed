#include "Renderer.h"

#include "Core.h"

Renderer::Renderer() {}

Renderer::~Renderer() {}

void Renderer::clear() const
{
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
}

void Renderer::setClearColor(const float r, const float g, const float b, const float a) const
{
    GL_CALL(glClearColor(r, g, b, a));
}

void Renderer::draw(const VertexArray &va, const Shader &shader) const
{
    shader.bind();
    va.bind();
    GL_CALL(glDrawElements(GL_TRIANGLES, va.getIndexCount(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::draw(const VertexArray &va, const Shader &shader, unsigned int drawCount, unsigned int indexOffset) const
{
    shader.bind();
    va.bind();
    GL_CALL(glDrawElements(GL_TRIANGLES, drawCount, GL_UNSIGNED_INT, (void *)(indexOffset * sizeof(unsigned int))));
}
