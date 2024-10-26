#include "Shader.h"
#include "Utilities/Logger.h"
#include <fstream>
#include <sstream>

Shader::Shader(std::filesystem::path sourcePath, std::filesystem::path binaryPath)
    : BaseShader(std::move(sourcePath), std::move(binaryPath)) {
    LoadShader();
}

void Shader::ReloadShader() {
    LoadShader();
}

void Shader::LoadShader() {
    if (m_RendererID) {
        glDeleteProgram(m_RendererID);
        m_RendererID = 0;
    }
    m_UniformLocationCache.clear();

    if (!m_BinaryPath.empty() && LoadBinary()) {
        Logger::GetLogger()->info("Loaded shader binary from '{}'.", m_BinaryPath.string());
        return;
    }

    // Read shader source
    std::string source = ReadFile(m_SourcePath);
    source = ResolveIncludes(source, m_SourcePath.parent_path());

    // Separate shader types
    enum class ShaderType { NONE, VERTEX, FRAGMENT };
    ShaderType type = ShaderType::NONE;
    std::unordered_map<ShaderType, std::stringstream> shaderSources;

    std::istringstream stream(source);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
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

    if (shaderSources.find(ShaderType::VERTEX) == shaderSources.end() ||
        shaderSources.find(ShaderType::FRAGMENT) == shaderSources.end()) {
        throw std::runtime_error("Shader source does not contain both vertex and fragment shaders.");
    }

    // Compile shaders
    std::vector<GLuint> shaders;
    shaders.push_back(CompileShader(GL_VERTEX_SHADER, shaderSources[ShaderType::VERTEX].str()));
    shaders.push_back(CompileShader(GL_FRAGMENT_SHADER, shaderSources[ShaderType::FRAGMENT].str()));

    // Link program
    m_RendererID = LinkProgram(shaders);

    // Save binary if binary path is provided
    if (!m_BinaryPath.empty()) {
        SaveBinary();
    }
}