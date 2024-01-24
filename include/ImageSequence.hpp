#pragma once

#include <string>
#include <vector>

class ImageSequence
{
public:
    bool open(const std::string& dir);

    int getCount() const;

    const std::string& getFilename(int i) const;

private:
    std::vector<std::string> m_fnames;
};
