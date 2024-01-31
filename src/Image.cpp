#include <Image.hpp>
#include <iostream>
#include <algorithm>
#include <ImageJpegIo.hpp>
#include <ImagePngIo.hpp>
#include <ImageTiffIo.hpp>

Image::Image():
    m_width(0),
    m_height(0)
{

}

void Image::create(int width, int height)
{
    m_width = width;
    m_height = height;
    m_data.resize(width * height * 4, 0.f);
}

bool Image::load(const std::string& filename)
{
    ImgFormat format;
    if (!parseFormat(filename, format))
        return false;

    bool ret = false;
    if (format == ImgFormat::Jpeg)
    {
        ImageJpegIo io;
        ret = io.load(filename, *this);
    }
    else if (format == ImgFormat::Png)
    {
        ImagePngIo io;
        ret = io.load(filename, *this);
    }
    else if (format == ImgFormat::Tiff)
    {
        ImageTiffIo io;
        ret = io.load(filename, *this);
    }

    if (!ret)
        std::cerr << "Error: Could not load the image " << filename << std::endl;

    return ret;
}

bool Image::save(const std::string& filename) const
{
    ImgFormat format;
    if (!parseFormat(filename, format))
        return false;

    bool ret = false;
    if (format == ImgFormat::Jpeg)
    {
        ImageJpegIo io;
        ret = io.save(filename, *this);
    }
    else if (format == ImgFormat::Png)
    {
        ImagePngIo io;
        ret = io.save(filename, *this);
    }
    else if (format == ImgFormat::Tiff)
    {
        ImageTiffIo io;
        ret = io.save(filename, *this);
    }

    if (!ret)
        std::cerr << "Error: Could not save the image " << filename << std::endl;

    return ret;
}

float Image::getSample(int x, int y, int channel) const
{
    return m_data[(x + y * m_width) * 4 + channel];
}

void Image::setSample(int x, int y, int channel, float sample)
{
    m_data[(x + y * m_width) * 4 + channel] = sample;
}

bool Image::parseFormat(const std::string& filename, ImgFormat& format) const
{
    auto extPos = filename.rfind(".");
    if (extPos == std::string::npos)
    {
        std::cerr << "Error: Can not determine the format of image " << filename << std::endl;
        return false;
    }

    std::string ext = filename.substr(extPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });

    if (ext.compare(".jpg") == 0 || ext.compare(".jpeg") == 0)
        format = ImgFormat::Jpeg;
    else if (ext.compare(".png") == 0)
        format = ImgFormat::Png;
    else if (ext.compare(".tif") == 0 || ext.compare(".tiff") == 0)
        format = ImgFormat::Tiff;
    else
    {
        std::cerr << "Error: Can not determine the format of image " << filename << std::endl;
        return false;
    }

    return true;
}

int Image::getWidth() const
{
    return m_width;
}

int Image::getHeight() const
{
    return m_height;
}
