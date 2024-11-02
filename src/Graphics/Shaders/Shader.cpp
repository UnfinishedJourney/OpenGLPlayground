#include "Shader.h"
#include "Utilities/Logger.h"
#include <fstream>
#include <sstream>

Shader::Shader(std::filesystem::path sourcePath, std::filesystem::path binaryPath)
    : BaseShader(std::move(sourcePath), std::move(binaryPath)) {
    LoadShader();
}

void Shader::ReloadShader() {
    if (m_RendererID) {
        glDeleteProgram(m_RendererID);
        m_RendererID = 0;
    }
    m_UniformLocationCache.clear();
    LoadShader();
}

void Shader::LoadShader(bool bReload) {
    if (!bReload && !m_BinaryPath.empty() && LoadBinary()) {
        Logger::GetLogger()->info("Loaded shader binary from '{}'.", m_BinaryPath.string());
        return;
    }

    // Read shader source
    std::string source = ReadFile(m_SourcePath);

    // Separate shader types
    enum class ShaderType { NONE, VERTEX, FRAGMENT };
    ShaderType type = ShaderType::NONE;
    std::unordered_map<ShaderType, std::string> shaderSources;

    std::istringstream stream(source);
    std::string line;

    std::ostringstream vertexShaderStream;
    std::ostringstream fragmentShaderStream;

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
            if (type == ShaderType::VERTEX) {
                vertexShaderStream << line << '\n';
            }
            else if (type == ShaderType::FRAGMENT) {
                fragmentShaderStream << line << '\n';
            }
        }
    }

    if (vertexShaderStream.str().empty() || fragmentShaderStream.str().empty()) {
        throw std::runtime_error("Shader source does not contain both vertex and fragment shaders.");
    }

    // Process vertex shader includes
    std::unordered_set<std::string> vertexIncludedFiles;
    std::string vertexSource = ResolveIncludes(vertexShaderStream.str(), m_SourcePath.parent_path(), vertexIncludedFiles);

    // Process fragment shader includes
    std::unordered_set<std::string> fragmentIncludedFiles;
    std::string fragmentSource = ResolveIncludes(fragmentShaderStream.str(), m_SourcePath.parent_path(), fragmentIncludedFiles);

    // Compile shaders
    std::vector<GLuint> shaders;
    shaders.push_back(CompileShader(GL_VERTEX_SHADER, vertexSource));
    shaders.push_back(CompileShader(GL_FRAGMENT_SHADER, fragmentSource));

    // Link program
    m_RendererID = LinkProgram(shaders);

    // Save binary if binary path is provided
    if (!m_BinaryPath.empty()) {
        SaveBinary();
    }
}