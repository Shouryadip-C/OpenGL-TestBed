#include "Renderer.h"

#include "Core.h"

Renderer::Renderer() {}

Renderer::~Renderer() {}

void Renderer::clear()
{
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Renderer::setClearColor(const float r, const float g, const float b, const float a)
{
    GL_CALL(glClearColor(r, g, b, a));
}

void Renderer::draw(const VertexArray &va, const Shader &shader)
{
    shader.bind();
    va.bind();
    if (va.getIndexCount()) {
        GL_CALL(glDrawElements(GL_TRIANGLES, va.getIndexCount(), GL_UNSIGNED_INT, nullptr));
    }
    else {
        GL_CALL(glDrawArrays(GL_TRIANGLES, 0, va.getVertexCount()));
    }
}

void Renderer::draw(const VertexArray &va, const Shader &shader, unsigned int drawCount, unsigned int startIndex)
{
    shader.bind();
    va.bind();
    if (va.getIndexCount()) {
        GL_CALL(glDrawElements(GL_TRIANGLES, drawCount, GL_UNSIGNED_INT, (void *)(startIndex * sizeof(unsigned int))));
    }
    else {
        GL_CALL(glDrawArrays(GL_TRIANGLES, startIndex, drawCount));
    }
}
