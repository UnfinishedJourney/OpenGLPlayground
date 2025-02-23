#include "Shader.h"
#include "Utilities/Logger.h"
#include <stdexcept>
#include <unordered_set>

namespace graphics {

    Shader::Shader(const std::unordered_map<GLenum, std::filesystem::path>& shaderStages,
        const std::filesystem::path& binaryPath)
        : BaseShader(binaryPath), shaderStages_(shaderStages)
    {
        LoadShaders(false);
    }

    void Shader::ReloadShader() {
        if (rendererId_ != 0) {
            glDeleteProgram(rendererId_);
            rendererId_ = 0;
        }
        uniformLocationCache_.clear();
        LoadShaders(true);
    }

    void Shader::LoadShaders(bool reload) {
        if (!reload && !binaryPath_.empty() && LoadBinary()) {
            Logger::GetLogger()->info("Loaded shader program from binary '{}'.", binaryPath_.string());
            return;
        }
        std::vector<GLuint> compiledShaders;
        compiledShaders.reserve(shaderStages_.size());
        try {
            for (const auto& [type, path] : shaderStages_) {
                std::string src = ReadFile(path);
                std::unordered_set<std::string> included;
                src = ResolveIncludes(src, path.parent_path(), included);
                compiledShaders.push_back(CompileShader(type, src));
            }
            rendererId_ = LinkProgram(compiledShaders);
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("Error loading shaders: {}", e.what());
            throw;
        }
        if (rendererId_ == 0) {
            Logger::GetLogger()->error("Failed to link shader program from sources.");
            return;
        }
        if (!binaryPath_.empty()) {
            try {
                SaveBinary();
            }
            catch (const std::exception& e) {
                Logger::GetLogger()->error("Failed to save shader binary: {}", e.what());
            }
        }
        Logger::GetLogger()->info("Shader program created successfully, ID={}.", rendererId_);
    }

} // namespace graphics
