#include "TextureData.h"
#include <stb_image.h>
#include <cstring>

namespace graphics {

    bool TextureData::LoadFromFile(const std::string& filePath,
        bool flipY,
        bool force4Ch,
        bool isHDR)
    {
        m_IsHDR = isHDR;
        stbi_set_flip_vertically_on_load(flipY ? 1 : 0);

        if (m_IsHDR) {
            float* data = nullptr;
            if (force4Ch) {
                data = stbi_loadf(filePath.c_str(), &m_Width, &m_Height, &m_Channels, 4);
                if (data) {
                    m_Channels = 4;
                }
            }
            else {
                data = stbi_loadf(filePath.c_str(), &m_Width, &m_Height, &m_Channels, 0);
            }
            if (!data) {
                return false;
            }
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
                return false;
            }
            if (force4Ch) {
                m_Channels = 4;
            }
            size_t size = static_cast<size_t>(m_Width) * m_Height * m_Channels;
            m_ByteData.resize(size);
            std::memcpy(m_ByteData.data(), dataU8, size);
            stbi_image_free(dataU8);
        }

        stbi_set_flip_vertically_on_load(0);
        return true;
    }

} // namespace graphics