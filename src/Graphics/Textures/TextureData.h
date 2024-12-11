#pragma once
#include <string>
#include <vector>

class TextureData {
public:
    bool LoadFromFile(const std::string& filePath, bool flipY = true);
    const unsigned char* GetData() const { return m_Data.data(); }
    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    int GetChannels() const { return m_Channels; }

private:
    std::vector<unsigned char> m_Data;
    int m_Width = 0;
    int m_Height = 0;
    int m_Channels = 0;
};