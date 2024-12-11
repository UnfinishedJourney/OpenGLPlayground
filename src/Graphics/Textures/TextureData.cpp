#include "TextureData.h"
#include <stb_image.h>
#include <cstring>

bool TextureData::LoadFromFile(const std::string& filePath, bool flipY) {
    stbi_set_flip_vertically_on_load(flipY ? 1 : 0);
    unsigned char* data = stbi_load(filePath.c_str(), &m_Width, &m_Height, &m_Channels, 4);
    if (!data) return false;
    size_t size = static_cast<size_t>(m_Width * m_Height * 4);
    m_Data.resize(size);
    std::memcpy(m_Data.data(), data, size);
    stbi_image_free(data);
    m_Channels = 4;
    return true;
}