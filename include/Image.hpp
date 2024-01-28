#pragma once

#include <string>
#include <vector>

class Image
{
public:
    Image();

    void create(int width, int height, int channels = 4);
    bool load(const std::string& filename);

    bool save(const std::string& filename) const;

    float getSample(int x, int y, int channel) const;
    void setSample(int x, int y, int channel, float sample);

    int getWidth() const;
    int getHeight() const;
    int getChannels() const;

private:
    enum class ImgFormat
    {
        Jpeg, Png, Tiff
    };

    bool parseFormat(const std::string& filename, ImgFormat& format) const;

    std::vector<float> m_data;
    int m_width;
    int m_height;
    int m_channels;
};