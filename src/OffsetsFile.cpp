#include <OffsetsFile.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <climits>
#include <Vector.hpp>

void OffsetsFile::create(int count)
{
    m_offsets.resize(count, { 0, 0 });
}

bool OffsetsFile::load(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open the offsets file " << filename;
        return false;
    }

    // Keep track of current index for interpolation
    int curI = 0;

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream ss(line);

        int i, x, y;
        // Do not treat as an error as it could have been just a blank line
        if (!(ss >> x >> y))
            continue;

        if (ss >> i)
        {
            // Correct the order of read data from xyi to ixy (xyi > iyx > ixy)
            std::swap(x, i);
            std::swap(y, x);

            if (m_offsets.size() <= i)
                m_offsets.resize(i + 1);

            m_offsets[i] = { x, y };

            // Linearly interpolate all offset vectors between last and current
            // keyframe
            if (curI == 0)
            {
                if (i != 0)
                {
                    std::cerr << "Error: First offset keyframe must be index 0" << std::endl;
                    return false;
                }
            }
            else
            {
                int interpBeg = curI - 1;
                int interpEnd = i;
                Vector interpBegVec = m_offsets[interpBeg];
                for (int j = interpBeg + 1; j < interpEnd; j++)
                {
                    float alpha = (float)(j - interpBeg) / (float)(interpEnd - interpBeg);
                    m_offsets[j] = {
                        (int)((float)interpBegVec.x * (1.f - alpha) + (float)x * alpha),
                        (int)((float)interpBegVec.y * (1.f - alpha) + (float)y * alpha)
                    };
                }
            }

            curI = i + 1;
        }
        else
        {
            if (m_offsets.size() <= curI)
                m_offsets.resize(curI + 1);
            m_offsets[curI] = { x, y };
            curI++;
        }
    }

    return true;
}

bool OffsetsFile::save(const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not save the offsets file " << filename;
        return false;
    }

    for (const auto& o : m_offsets)
        file << o.x << ' ' << o.y << std::endl;

    if (!file.good())
        std::cerr << "Error: Could not finish saving the offsets file " << filename;

    return file.good();
}

int OffsetsFile::getCount() const
{
    return m_offsets.size();
}

const Vector& OffsetsFile::get(int i) const
{
    return m_offsets[i];
}

void OffsetsFile::set(int i, Vector vec)
{
    m_offsets[i] = vec;
}

int OffsetsFile::findMinX() const
{
    int val = INT_MAX;
    for (const auto& v : m_offsets)
        val = std::min(val, v.x);
    return val;
}

int OffsetsFile::findMinY() const
{
    int val = INT_MAX;
    for (const auto& v : m_offsets)
        val = std::min(val, v.y);
    return val;
}

int OffsetsFile::findMaxX() const
{
    int val = INT_MIN;
    for (const auto& v : m_offsets)
        val = std::max(val, v.x);
    return val;
}

int OffsetsFile::findMaxY() const
{
    int val = INT_MIN;
    for (const auto& v : m_offsets)
        val = std::max(val, v.y);
    return val;
}
