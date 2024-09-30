#include "Shader.h"
#include <glad/glad.h>

#include <iostream>

void Shader::LoadShader(const std::filesystem::path& filepath) {

    if (m_RendererID) {
        glDeleteProgram(m_RendererID);
        m_RendererID = 0;
    }

    std::filesystem::path binaryPath = filepath;
    binaryPath.replace_extension(".bin");

    if (!LoadBinary(binaryPath)) {
        ReloadShader();
    }

}

void Shader::ReloadShader() {
    if (m_RendererID) {
        glDeleteProgram(m_RendererID);
        m_RendererID = 0;
    }

    try {
        std::string source = ReadFile(m_Filepath);
        source = ResolveIncludes(source, m_Filepath.parent_path());

        // Parse the shader source to separate different shader stages
        enum class ShaderType { NONE, VERTEX, FRAGMENT };
        ShaderType type = ShaderType::NONE;
        std::unordered_map<ShaderType, std::stringstream> shaderSources;

        std::istringstream stream(source);
        std::string line;

        while (std::getline(stream, line)) {
            if (line.find("#shader") != std::string::npos) {
                // Determine the shader type
                if (line.find("vertex") != std::string::npos) {
                    type = ShaderType::VERTEX;
                }
                else if (line.find("fragment") != std::string::npos) {
                    type = ShaderType::FRAGMENT;
                }
                else {
                    type = ShaderType::NONE;
                }
            }
            else {
                if (type != ShaderType::NONE) {
                    shaderSources[type] << line << '\n';
                }
            }
        }

        // Compile shaders
        std::vector<unsigned int> shaders;

        if (shaderSources.find(ShaderType::VERTEX) != shaderSources.end()) {
            shaders.push_back(CompileShader(GL_VERTEX_SHADER, shaderSources[ShaderType::VERTEX].str()));
        }
        else {
            throw std::runtime_error("Vertex shader not found in file: " + m_Filepath.string());
        }

        if (shaderSources.find(ShaderType::FRAGMENT) != shaderSources.end()) {
            shaders.push_back(CompileShader(GL_FRAGMENT_SHADER, shaderSources[ShaderType::FRAGMENT].str()));
        }
        else {
            throw std::runtime_error("Fragment shader not found in file: " + m_Filepath.string());
        }

        // Link program
        m_RendererID = LinkProgram(shaders);

        std::filesystem::path binaryPath = m_Filepath;
        binaryPath.replace_extension(".bin");

        // Save the binary for future use
        SaveBinary(binaryPath);
    }
    catch (const std::exception& e) {
        std::cerr << "Error reloading shader: " << e.what() << std::endl;
    }
}