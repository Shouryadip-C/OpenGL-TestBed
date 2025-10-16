#pragma once

#include "Mesh.h"
#include "Texture.h"
#include <assimp/scene.h>

#include <filesystem>
#include <vector>


class Model
{
public:
    std::vector<MeshTexture> textures_loaded;
    std::vector<Mesh>        meshes;
    std::string              directory;
    bool                     gammaCorrection;

    // constructor, expects a filepath to a 3D model.
    Model(const std::filesystem::path &path, bool gamma = false);

    // draws the model, and thus all its meshes
    void draw(Shader &shader);

private:
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(const std::filesystem::path &path);

    void processNode(aiNode *node, const aiScene *scene);

    Mesh processMesh(aiMesh *mesh, const aiScene *scene);

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    std::vector<MeshTexture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};
