#pragma once

#include <string>
#include <vector>

struct Vector;

class OffsetsFile
{
public:
    void create(int count);

    bool load(const std::string& filename);
    bool save(const std::string& filename);

    int getCount() const;

    const Vector& get(int i) const;
    void set(int i, Vector vec);

    int findMinX() const;
    int findMinY() const;
    int findMaxX() const;
    int findMaxY() const;

private:
    std::vector<Vector> m_offsets;
};
