#include "Shader.h"
#include "Utilities/Logger.h"
#include <stdexcept>
#include <unordered_set>

Shader::Shader(const std::unordered_map<GLenum, std::filesystem::path>& shaderStages,
    const std::filesystem::path& binaryPath)
    : BaseShader(binaryPath)
    , m_ShaderStages(shaderStages)
{
    LoadShaders(false);
}

void Shader::ReloadShader()
{
    if (*m_RendererIDPtr != 0) {
        glDeleteProgram(*m_RendererIDPtr);
        *m_RendererIDPtr = 0;
    }
    m_UniformLocationCache.clear();
    LoadShaders(true);
}

void Shader::LoadShaders(bool reload)
{
    // If not reloading and we have a binary path, try loading from binary
    if (!reload && !m_BinaryPath.empty() && LoadBinary()) {
        Logger::GetLogger()->info("Loaded shader program from binary '{}'.", m_BinaryPath.string());
        return;
    }

    std::vector<GLuint> compiled;
    compiled.reserve(m_ShaderStages.size());
    try {
        for (const auto& [type, path] : m_ShaderStages) {
            std::string src = ReadFile(path);
            std::unordered_set<std::string> includes;
            src = ResolveIncludes(src, path.parent_path(), includes);
            compiled.push_back(CompileShader(type, src));
        }

        *m_RendererIDPtr = LinkProgram(compiled);
    }
    catch (const std::exception& e) {
        Logger::GetLogger()->error("Error loading shaders: {}", e.what());
        throw;
    }

    if (*m_RendererIDPtr == 0) {
        Logger::GetLogger()->error("Failed to link shader program from sources.");
        return;
    }

    // Optionally save the binary
    if (!m_BinaryPath.empty()) {
        try {
            SaveBinary();
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("Failed to save shader binary: {}", e.what());
        }
    }

    Logger::GetLogger()->info("Shader program created successfully, ID={}.", *m_RendererIDPtr);
}

void Shader::BindUniformBlock(const std::string& blockName, GLuint bindingPoint)
{
    GLuint blockIndex = glGetUniformBlockIndex(*m_RendererIDPtr, blockName.c_str());
    if (blockIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(*m_RendererIDPtr, blockIndex, bindingPoint);
        Logger::GetLogger()->debug(
            "Bound uniform block '{}' to binding point {} in shader ID {}.",
            blockName, bindingPoint, *m_RendererIDPtr
        );
    }
    else {
        Logger::GetLogger()->warn(
            "Uniform block '{}' not found in shader program ID {}.",
            blockName, *m_RendererIDPtr
        );
    }
}

void Shader::BindShaderStorageBlock(const std::string& blockName, GLuint bindingPoint)
{
    GLuint blockIndex = glGetProgramResourceIndex(*m_RendererIDPtr, GL_SHADER_STORAGE_BLOCK, blockName.c_str());
    if (blockIndex != GL_INVALID_INDEX) {
        glShaderStorageBlockBinding(*m_RendererIDPtr, blockIndex, bindingPoint);
        Logger::GetLogger()->debug(
            "Bound shader storage block '{}' to binding point {} in shader ID {}.",
            blockName, bindingPoint, *m_RendererIDPtr
        );
    }
    else {
        Logger::GetLogger()->warn(
            "Shader storage block '{}' not found in shader program ID {}.",
            blockName, *m_RendererIDPtr
        );
    }
}