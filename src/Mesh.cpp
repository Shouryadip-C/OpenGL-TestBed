#include "Mesh.h"

#include "Renderer.h"


Mesh::Mesh(std::vector<MeshVertex> vertices, std::vector<unsigned int> indices, std::vector<MeshTexture> textures)
{
    this->vertices = vertices;
    this->indices  = indices;
    this->textures = textures;

    m_VAO          = std::make_unique<VertexArray>();
    m_vertexBuffer = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(MeshVertex));
    m_indexBuffer  = std::make_unique<IndexBuffer>(indices.data(), indices.size());

    VertexBufferLayout layout;
    layout.push<float>(3);                   // position
    layout.push<float>(3);                   // normal
    layout.push<float>(2);                   // tex coords
    layout.push<float>(3);                   // tangent
    layout.push<float>(3);                   // bitangent
    layout.push<int>(MAX_BONE_INFLUENCE);    // bone indices
    layout.push<float>(MAX_BONE_INFLUENCE);  // weights

    m_VAO->addBuffer(*m_vertexBuffer, layout);
    m_VAO->addBuffer(*m_indexBuffer);
}

void Mesh::draw(Shader &shader)
{
    unsigned int diffuseNr  = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr   = 1;
    unsigned int heightNr   = 1;
    for (unsigned int i = 0; i < textures.size(); i++) {
        std::string number;
        std::string name = textures[i].type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);
        else if (name == "texture_normal")
            number = std::to_string(normalNr++);
        else if (name == "texture_height")
            number = std::to_string(heightNr++);

        shader.setUniform1i((name + number).c_str(), i);
        textures[i].texture->bind(i);
    }

    Renderer::draw(*m_VAO, shader);
}
