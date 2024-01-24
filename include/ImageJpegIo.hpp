#pragma once

#include <string>

class Image;

class ImageJpegIo
{
public:
    bool load(const std::string& filename, Image& image);
    bool save(const std::string& filename, const Image& image);
};
