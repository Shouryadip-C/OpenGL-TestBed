#pragma once

#include "Core.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include <glm/glm.hpp>

#include <string>
#include <vector>

#define MAX_BONE_INFLUENCE 4

struct MeshVertex
{
    float position[3];
    float normal[3];
    float texCoords[2];
    float tangent[3];
    float bitangent[3];
    //bone indexes which will influence this MeshVertex
    int boneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float weights[MAX_BONE_INFLUENCE];
};

struct MeshTexture
{
    std::shared_ptr<Texture> texture;
    std::string              type;
};

class Mesh
{
public:
    std::vector<MeshVertex>   vertices;
    std::vector<unsigned int> indices;
    std::vector<MeshTexture>  textures;

    Mesh(std::vector<MeshVertex> vertices, std::vector<unsigned int> indices, std::vector<MeshTexture> textures);

    void draw(Shader &shader);

private:
    std::unique_ptr<VertexBuffer> m_vertexBuffer;
    std::unique_ptr<IndexBuffer>  m_indexBuffer;
    std::unique_ptr<VertexArray>  m_VAO;
};
