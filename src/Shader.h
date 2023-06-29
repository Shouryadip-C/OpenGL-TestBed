#pragma once

#include <unordered_map>

#include <filesystem>
#include <string>


struct ShaderProgramSource
{
    std::string vertexSource;
    std::string fragmentSource;
};

class Shader
{
private:
    std::filesystem::path                m_filePath;
    unsigned int                         m_rendererID;
    std::unordered_map<std::string, int> m_uniformLocationCache;

    int          getUniformLocation(const std::string &name);
    unsigned int compile(const unsigned int type, const std::string &shaderSource);
    unsigned int createProgram(const std::string &vertexSource, const std::string &fragmentSource);

    ShaderProgramSource loadFromFile(const std::filesystem::path &path);

public:
    Shader(const std::filesystem::path &filePath);
    ~Shader();

    void bind() const;
    void unbind() const;

    // set uniforms
    void setUniform1i(const std::string &name, const int i);
    void setUniform1f(const std::string &name, const float f);
    void setUniform4f(const std::string &name, const float v0, const float v1, const float v2, const float v3);
};
