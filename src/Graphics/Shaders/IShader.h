#pragma once
#include <string_view>
#include <glm/glm.hpp>

namespace Graphics {

    /**
     * @brief Interface for shader programs.
     *
     * Declares the basic operations that every shader must support.
     */
    class IShader {
    public:
        virtual ~IShader() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
        virtual void ReloadShader() = 0;

        // Uniform setters.
        virtual void SetUniform(std::string_view name, float value) const = 0;
        virtual void SetUniform(std::string_view name, int value) const = 0;
        virtual void SetUniform(std::string_view name, unsigned int value) const = 0;
        virtual void SetUniform(std::string_view name, const glm::vec2& value) const = 0;
        virtual void SetUniform(std::string_view name, const glm::vec3& value) const = 0;
        virtual void SetUniform(std::string_view name, const glm::vec4& value) const = 0;
        virtual void SetUniform(std::string_view name, const glm::mat3& value) const = 0;
        virtual void SetUniform(std::string_view name, const glm::mat4& value) const = 0;
    };

} // namespace Graphics