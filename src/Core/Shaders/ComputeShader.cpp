#include "ComputeShader.h"
#include <glad/glad.h>

void ComputeShader::LoadShader(const std::filesystem::path& filepath) {
    std::string source = ReadFile(filepath);
    source = ResolveIncludes(source, filepath.parent_path());

    // Parsing shader source code using #shader directives
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
        throw std::runtime_error("Compute shader not found in file: " + filepath.string());
    }

    // Compile shader
    unsigned int shader = CompileShader(GL_COMPUTE_SHADER, computeShaderSource.str());

    // Link program
    m_RendererID = LinkProgram({ shader });
}

void ComputeShader::Dispatch(unsigned int numGroupsX, unsigned int numGroupsY, unsigned int numGroupsZ) const {
    Bind();
    glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT);
    Unbind();
}