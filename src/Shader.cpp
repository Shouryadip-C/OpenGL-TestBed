#include "Shader.h"

#include "Core.h"

#include <fstream>
#include <iostream>
#include <sstream>


Shader::Shader(const std::filesystem::path &filePath) : m_filePath(filePath), m_rendererID(0)
{
    ShaderProgramSource source{ loadFromFile(filePath) };
    m_rendererID = createProgram(source.vertexSource, source.fragmentSource);
}


Shader::~Shader()
{
    GL_CALL(glDeleteProgram(m_rendererID));
}

void Shader::bind() const
{
    GL_CALL(glUseProgram(m_rendererID));
}


void Shader::unbind() const
{
    GL_CALL(glUseProgram(0));
}


int Shader::getUniformLocation(const std::string &name)
{
    if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end()) {
        return m_uniformLocationCache[name];
    }


    GL_CALL(int location{ glGetUniformLocation(m_rendererID, name.c_str()) });
    if (location == -1) {
        std::cerr << "Warning: uniform '" << name << "' not found in shader!"
                  << "\n";
    }

    // cache the location of the uniform so that we dont have to retrieve it again
    m_uniformLocationCache[name] = location;

    return location;
}


unsigned int Shader::compile(const unsigned int type, const std::string &source)
{
    unsigned int shaderID = glCreateShader(type);
    const char  *sourceCStr{ source.c_str() };
    GL_CALL(glShaderSource(shaderID, 1, &sourceCStr, NULL));
    GL_CALL(glCompileShader(shaderID));

    // error handling for shader compilation
    int success;
    GL_CALL(glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success));

    if (success == GL_FALSE) {
        std::cerr << "Error: " << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << " shader compilation failed!"
                  << "\n";

        int shaderInfoLogLength;
        GL_CALL(glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &shaderInfoLogLength));
        if (shaderInfoLogLength) {
            char *message = new char[shaderInfoLogLength];
            GL_CALL(glGetShaderInfoLog(shaderID, shaderInfoLogLength, NULL, message));
            std::cerr << message << "\n";
            delete[] message;
        }
        GL_CALL(glDeleteShader(shaderID));

        return 0;
    }

    return shaderID;
}


unsigned int Shader::createProgram(const std::string &vertexSource, const std::string &fragmentSource)
{
    GL_CALL(unsigned int program{ glCreateProgram() });
    unsigned int vertexShader{ compile(GL_VERTEX_SHADER, vertexSource) };
    unsigned int fragmentShader{ compile(GL_FRAGMENT_SHADER, fragmentSource) };

    GL_CALL(glAttachShader(program, vertexShader));
    GL_CALL(glAttachShader(program, fragmentShader));
    GL_CALL(glLinkProgram(program));

    // error handling shader program linkage and validation
    int success;
    GL_CALL(glGetProgramiv(program, GL_LINK_STATUS, &success));
    if (!success) {
        std::cout << "Error: Shader program creation failed!"
                  << "\n";

        int programInfoLogLength;
        GL_CALL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &programInfoLogLength));
        if (programInfoLogLength) {
            char *message = new char[programInfoLogLength];
            GL_CALL(glGetProgramInfoLog(program, programInfoLogLength, NULL, message));
            std::cout << message << std::endl;
            delete[] message;
        }
        GL_CALL(glDeleteProgram(program));
        GL_CALL(glDeleteShader(vertexShader));
        GL_CALL(glDeleteShader(fragmentShader));

        return 0;
    }

    // TODO: Check if program is valid for the current opengl state
    // cannot do this in the above success condition check as the program info log
    // is overwritten by glValidateProgram

    // int isValid;
    // GL_CALL(glValidateProgram(program));
    // GL_CALL(glGetProgramiv(program, GL_VALIDATE_STATUS, &isValid));

    GL_CALL(glDeleteShader(vertexShader));
    GL_CALL(glDeleteShader(fragmentShader));

    return program;
}


ShaderProgramSource Shader::loadFromFile(const std::filesystem::path &path)
{
    std::ifstream file(path);

    if (!file) {
        std::cerr << "File not found: " << path << "\n";
        return {};
    }

    std::string       line;
    std::stringstream ss[2];

    enum ShaderType { None = -1, Vertex = 0, Fragment = 1 };
    ShaderType type{ ShaderType::None };

    while (std::getline(file, line)) {
        if (line.find("//shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                type = ShaderType::Vertex;
            }
            else if (line.find("fragment") != std::string::npos) {
                type = ShaderType::Fragment;
            }
            else if (line.find("none") != std::string::npos) {
                type = ShaderType::None;
            }
        }
        else if (type != ShaderType::None) {
            ss[static_cast<int>(type)] << line << "\n";
        }
    }

    return { ss[0].str(), ss[1].str() };
}


// *** Uniforms

void Shader::setUniform1i(const std::string &name, const int i)
{
    GL_CALL(glUniform1i(getUniformLocation(name), i));
}

void Shader::setUniform1f(const std::string &name, const float f)
{
    GL_CALL(glUniform1f(getUniformLocation(name), f));
}

void Shader::setUniform4f(const std::string &name, const float v0, const float v1, const float v2, const float v3)
{
    GL_CALL(glUniform4f(getUniformLocation(name), v0, v1, v2, v3));
}

void Shader::setUniformMat4f(const std::string &name, const int count, const bool transpose, const float *data)
{
    GL_CALL(glUniformMatrix4fv(getUniformLocation(name), count, transpose, data));
}
