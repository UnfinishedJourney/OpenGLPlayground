#pragma once
#include <string>
#include <vector>

namespace Graphics {

    /**
     * @brief Loads image data from disk (either LDR or HDR).
     *
     * Optionally flips the image vertically and can force four channels.
     */
    class TextureData {
    public:
        /**
         * @brief Loads texture data from a file.
         * @param filePath Path to the image.
         * @param flipY If true, flips the image vertically.
         * @param force4Ch Forces loading 4 channels (RGBA) if true.
         * @param isHDR Loads as HDR (float) if true.
         * @return true on success, false otherwise.
         */
        bool LoadFromFile(const std::string& filePath,
            bool flipY = true,
            bool force4Ch = true,
            bool isHDR = false);

        /// Returns LDR (8-bit) pixel data if available.
        const unsigned char* GetDataU8() const { return m_ByteData.empty() ? nullptr : m_ByteData.data(); }
        /// Returns HDR (float) pixel data if available.
        const float* GetDataFloat() const { return m_FloatData.empty() ? nullptr : m_FloatData.data(); }

        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }
        int GetChannels() const { return m_Channels; }
        bool IsHDR() const { return m_IsHDR; }

    private:
        std::vector<unsigned char> m_ByteData;   ///< LDR data.
        std::vector<float>         m_FloatData;  ///< HDR data.
        int m_Width = 0;
        int m_Height = 0;
        int m_Channels = 0;
        bool m_IsHDR = false;
    };

} // namespace Graphics