#include <ImageSequence.hpp>
#include <filesystem>
#include <iostream>
#include <algorithm>

bool ImageSequence::open(const std::string& dir)
{
    for (const auto& dirEntry : std::filesystem::directory_iterator(dir))
    {
        std::string ext = dirEntry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
        if (ext.compare(".jpg") == 0 || ext.compare(".jpeg") == 0 || ext.compare(".png") == 0 || ext.compare(".tif") == 0 || ext.compare(".tiff") == 0)
            m_fnames.push_back(dirEntry.path().string());
    }

    // TODO: Numerical sort to avoid sequences like 0.jpg, 1.jpg, 10.jpg etc.
    std::sort(m_fnames.begin(), m_fnames.end());

    return true;
}

int ImageSequence::getCount() const
{
    return m_fnames.size();
}

const std::string& ImageSequence::getFilename(int i) const
{
    return m_fnames[i];
}
