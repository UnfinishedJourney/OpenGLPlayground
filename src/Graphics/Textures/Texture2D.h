#pragma once

#include "Graphics/Textures/TextureBase.h"
#include <string_view>

class Texture2D : public TextureBase {
public:
    Texture2D() = default;
    explicit Texture2D(std::filesystem::path filePath);

    ~Texture2D() override = default;

    Texture2D(const Texture2D& other) = delete;
    Texture2D& operator=(const Texture2D& other) = delete;
    Texture2D(Texture2D&& other) noexcept = default;
    Texture2D& operator=(Texture2D&& other) noexcept = default;

    void Bind(GLuint slot = 0) const override;
    void Unbind(GLuint slot = 0) const override;

private:
    void LoadTexture();
};