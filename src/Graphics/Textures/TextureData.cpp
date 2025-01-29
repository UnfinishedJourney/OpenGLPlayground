#include "TextureData.h"
#include <stb_image.h>
#include <cstring>

bool TextureData::LoadFromFile(const std::string& filePath,
    bool flipY,
    bool force4Ch,
    bool isHDR)
{
    m_IsHDR = isHDR;
    stbi_set_flip_vertically_on_load(flipY ? 1 : 0);

    // Decide whether we call stbi_loadf (for HDR) or stbi_load (for LDR).
    if (m_IsHDR) {
        float* data = nullptr;
        if (force4Ch) {
            // Force 4 channels
            data = stbi_loadf(filePath.c_str(), &m_Width, &m_Height, &m_Channels, 4);
            if (data) {
                m_Channels = 4; // We forced 4
            }
        }
        else {
            // Load channels as-is
            data = stbi_loadf(filePath.c_str(), &m_Width, &m_Height, &m_Channels, 0);
        }

        if (!data) {
            return false; // stbi_loadf failed
        }

        // Copy data into m_FloatData
        size_t size = static_cast<size_t>(m_Width) * m_Height * m_Channels;
        m_FloatData.resize(size);
        std::memcpy(m_FloatData.data(), data, size * sizeof(float));
        stbi_image_free(data);
    }
    else {
        unsigned char* dataU8 = nullptr;
        int reqChannels = force4Ch ? 4 : 0;
        dataU8 = stbi_load(filePath.c_str(), &m_Width, &m_Height, &m_Channels, reqChannels);
        if (!dataU8) {
            return false; // stbi_load failed
        }

        if (force4Ch) {
            m_Channels = 4; // We forced 4
        }

        size_t size = static_cast<size_t>(m_Width) * m_Height * m_Channels;
        m_ByteData.resize(size);
        std::memcpy(m_ByteData.data(), dataU8, size);
        stbi_image_free(dataU8);
    }

    return true;
}