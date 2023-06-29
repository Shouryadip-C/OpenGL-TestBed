#pragma once

#include "IndexBuffer.h"
#include "Shader.h"
#include "VertexArray.h"

class Renderer
{
private:

public:
    Renderer();
    ~Renderer();

    static void clear();
    static void setClearColor(const float r, const float g, const float b, const float a);
    static void draw(const VertexArray &va, const Shader &shader);
    static void draw(const VertexArray &va, const Shader &shader, unsigned int drawCount, unsigned int startIndex = 0);
};
