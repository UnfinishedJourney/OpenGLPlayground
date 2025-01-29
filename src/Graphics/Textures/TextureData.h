#pragma once

#include <string>
#include <vector>

/**
 * @brief Utility class to load image data (LDR or HDR) into CPU memory.
 *        Supports optional Y-flip on load.
 */
class TextureData {
public:
    /**
     * @brief Load texture from file path.
     * @param filePath The image file path.
     * @param flipY    If true, flip vertically.
     * @param force4Ch If true, forces 4 channels (RGBA).
     * @return True on success, false otherwise.
     */
    bool LoadFromFile(const std::string& filePath,
        bool flipY = true,
        bool force4Ch = true,
        bool isHDR = false);

    /**
     * @return Pointer to the raw pixel data in CPU memory (float if HDR, ubyte if not).
     */
    const unsigned char* GetDataU8() const { return m_ByteData.empty() ? nullptr : m_ByteData.data(); }

    /**
     * @return Pointer to float pixel data if HDR; otherwise nullptr.
     */
    const float* GetDataFloat() const { return m_FloatData.empty() ? nullptr : m_FloatData.data(); }

    int GetWidth()    const { return m_Width; }
    int GetHeight()   const { return m_Height; }
    int GetChannels() const { return m_Channels; }
    bool IsHDR()      const { return m_IsHDR; }

private:
    std::vector<unsigned char> m_ByteData;   ///< If loading LDR
    std::vector<float>         m_FloatData;  ///< If loading HDR

    int  m_Width = 0;
    int  m_Height = 0;
    int  m_Channels = 0;
    bool m_IsHDR = false;
};