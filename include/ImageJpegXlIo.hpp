#pragma once

#include <string>
#include <vector>

class Image;

class ImageJpegXlIo
{
public:
    bool load(const std::string& filename, Image& image);
    bool save(const std::string& filename, const Image& image);

private:
    bool loadFile(const std::string& filename, std::vector<char>& data);
    bool saveFile(const std::string& filename, const std::vector<char>& data);
};
