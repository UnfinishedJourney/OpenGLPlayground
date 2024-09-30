#include "ComputeShader.h"
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

void ComputeShader::LoadShader(const std::filesystem::path& filepath) {
    m_Filepath = filepath;

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

void ComputeShader::ReloadShader() {
    if (m_RendererID) {
        glDeleteProgram(m_RendererID);
        m_RendererID = 0;
    }

    try {
        std::string source = ReadFile(m_Filepath);
        source = ResolveIncludes(source, m_Filepath.parent_path());

        // Parse shader source code using #shader directives
        enum class ShaderType { NONE, COMPUTE };
        ShaderType type = ShaderType::NONE;
        std::stringstream computeShaderSource;

        std::istringstream stream(source);
        std::string line;

        while (std::getline(stream, line)) {
            // Remove carriage return character
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if (line.find("#shader") != std::string::npos) {
                // Determine the shader type
                if (line.find("compute") != std::string::npos) {
                    type = ShaderType::COMPUTE;
                }
                else {
                    type = ShaderType::NONE;
                }
                // Skip the #shader line
            }
            else if (type == ShaderType::COMPUTE) {
                computeShaderSource << line << '\n';
            }
        }

        if (computeShaderSource.str().empty()) {
            throw std::runtime_error("Compute shader not found in file: " + m_Filepath.string());
        }

        // Compile compute shader
        unsigned int shader = CompileShader(GL_COMPUTE_SHADER, computeShaderSource.str());

        // Link program
        m_RendererID = LinkProgram({ shader });

        std::filesystem::path binaryPath = m_Filepath;
        binaryPath.replace_extension(".bin");

        // Save the binary for future use
        SaveBinary(binaryPath);
    }
    catch (const std::exception& e) {
        std::cerr << "Error reloading compute shader: " << e.what() << std::endl;
    }
}

void ComputeShader::Dispatch(unsigned int numGroupsX, unsigned int numGroupsY, unsigned int numGroupsZ) const {
    Bind();
    glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT);
    Unbind();
}