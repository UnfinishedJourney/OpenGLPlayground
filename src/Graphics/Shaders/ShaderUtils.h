#pragma once

#include <string>
#include <filesystem>
#include <unordered_set>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include "Utilities/Logger.h"

namespace graphics {
    namespace shaderUtils {

        inline std::string ReadFile(const std::filesystem::path& filepath) {
            if (!std::filesystem::exists(filepath))
                throw std::runtime_error("Shader file does not exist: " + filepath.string());
            std::ifstream file(filepath);
            if (!file.is_open())
                throw std::runtime_error("Failed to open shader file: " + filepath.string());
            std::ostringstream oss;
            oss << file.rdbuf();
            return oss.str();
        }

        inline std::string ResolveIncludes(const std::string& source,
            const std::filesystem::path& directory,
            std::unordered_set<std::string>& includedFiles) {
            std::istringstream stream(source);
            std::ostringstream output;
            std::string line;
            bool inBlockComment = false;
            while (std::getline(stream, line)) {
                std::string originalLine = line;
                std::string trimmed = line;
                if (inBlockComment) {
                    size_t endPos = trimmed.find("*/");
                    if (endPos != std::string::npos) {
                        inBlockComment = false;
                        trimmed = trimmed.substr(endPos + 2);
                    }
                    else {
                        continue;
                    }
                }
                size_t startBlock = trimmed.find("/*");
                if (startBlock != std::string::npos) {
                    inBlockComment = true;
                    trimmed = trimmed.substr(0, startBlock);
                }
                size_t commentPos = trimmed.find("//");
                if (commentPos != std::string::npos)
                    trimmed = trimmed.substr(0, commentPos);
                while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\t'))
                    trimmed.pop_back();
                const std::string includeDirective = "#include";
                if (trimmed.find(includeDirective) != std::string::npos) {
                    size_t startQuote = trimmed.find_first_of("\"<");
                    size_t endQuote = trimmed.find_first_of("\">", startQuote + 1);
                    if (startQuote == std::string::npos || endQuote == std::string::npos)
                        throw std::runtime_error("Invalid #include syntax: " + originalLine);
                    std::string includeRelPath = trimmed.substr(startQuote + 1, endQuote - startQuote - 1);
                    auto includePath = (directory / includeRelPath).lexically_normal();
                    if (!includedFiles.count(includePath.string())) {
                        includedFiles.insert(includePath.string());
                        std::string includedSource = ReadFile(includePath);
                        includedSource = ResolveIncludes(includedSource, includePath.parent_path(), includedFiles);
                        output << includedSource << "\n";
                    }
                }
                else {
                    output << originalLine << "\n";
                }
            }
            return output.str();
        }

        inline GLenum GetShaderTypeFromString(const std::string& typeStr) {
            if (typeStr == "vertex")           return GL_VERTEX_SHADER;
            if (typeStr == "fragment")         return GL_FRAGMENT_SHADER;
            if (typeStr == "geometry")         return GL_GEOMETRY_SHADER;
            if (typeStr == "tess_control")     return GL_TESS_CONTROL_SHADER;
            if (typeStr == "tess_evaluation")  return GL_TESS_EVALUATION_SHADER;
            if (typeStr == "compute")          return GL_COMPUTE_SHADER;
            throw std::runtime_error("Unknown shader type string: " + typeStr);
        }

        inline std::filesystem::file_time_type GetLatestModificationTime(
            const std::filesystem::path& sourcePath,
            std::unordered_set<std::string>& processedFiles) {
            auto canonicalPath = std::filesystem::weakly_canonical(sourcePath).string();
            if (processedFiles.count(canonicalPath))
                return std::filesystem::file_time_type::min();
            processedFiles.insert(canonicalPath);
            if (!std::filesystem::exists(sourcePath))
                throw std::runtime_error("Missing shader file: " + sourcePath.string());
            auto latestTime = std::filesystem::last_write_time(sourcePath);
            std::ifstream file(sourcePath);
            if (!file)
                throw std::runtime_error("Could not open file: " + sourcePath.string());
            std::string line;
            bool inBlockComment = false;
            while (std::getline(file, line)) {
                std::string trimmed = line;
                if (inBlockComment) {
                    size_t endPos = trimmed.find("*/");
                    if (endPos != std::string::npos) {
                        inBlockComment = false;
                        trimmed = trimmed.substr(endPos + 2);
                    }
                    else {
                        continue;
                    }
                }
                if (auto startPos = trimmed.find("/*"); startPos != std::string::npos) {
                    inBlockComment = true;
                    trimmed = trimmed.substr(0, startPos);
                }
                if (auto commentPos = trimmed.find("//"); commentPos != std::string::npos)
                    trimmed = trimmed.substr(0, commentPos);
                while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\t'))
                    trimmed.pop_back();
                static const std::string includeDirective = "#include";
                if (trimmed.find(includeDirective) == 0) {
                    size_t startQuote = trimmed.find_first_of("\"<");
                    size_t endQuote = trimmed.find_first_of("\">", startQuote + 1);
                    if (startQuote == std::string::npos || endQuote == std::string::npos)
                        throw std::runtime_error("Invalid #include syntax in file: " + sourcePath.string());
                    std::string includeRelPath = trimmed.substr(startQuote + 1, endQuote - startQuote - 1);
                    auto includePath = sourcePath.parent_path() / includeRelPath;
                    includePath = std::filesystem::weakly_canonical(includePath);
                    if (std::filesystem::exists(includePath)) {
                        auto includeTime = GetLatestModificationTime(includePath, processedFiles);
                        if (includeTime > latestTime)
                            latestTime = includeTime;
                    }
                }
            }
            return latestTime;
        }

    } // namespace shaderUtils
} // namespace graphics