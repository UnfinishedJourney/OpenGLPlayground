#pragma once

#include "Graphics/Textures/TextureBase.h"
#include "Utilities/Utility.h"
#include "stb_image.h"
#include <glad/glad.h>
#include <string>
#include <iostream>
#include <stdexcept>

class Texture2D : public TextureBase
{
public:
    Texture2D();
    explicit Texture2D(const std::string& path);

    ~Texture2D() override = default;

    Texture2D(const Texture2D& other) = delete;
    Texture2D& operator=(const Texture2D& other) = delete;
    Texture2D(Texture2D&& other) noexcept = default;
    Texture2D& operator=(Texture2D&& other) noexcept = default;

    void Bind(unsigned int slot = 0) const override;
    void Unbind(unsigned int slot = 0) const override;

private:
    void LoadTexture(const std::string& path);
};