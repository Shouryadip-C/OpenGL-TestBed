#pragma once

#include "Shader.h"
#include "VertexArray.h"

class Renderer
{
private:

public:
    Renderer();
    ~Renderer();

    void clear() const;
    void setClearColor(const float r, const float g, const float b, const float a) const;
    void draw(const VertexArray &va, const Shader &shader) const;
    void draw(const VertexArray &va, const Shader &shader, unsigned int drawCount, unsigned int startIndex = 0) const;
};
