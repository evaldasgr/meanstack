#pragma once

#include <string>
#include <vector>

class FilenameComparator
{
public:
    static bool compare(const std::string& a, const std::string& b);

private:
    struct Part
    {
        bool digits = false;
        std::string str;
    };

    static void split(const std::string& str, std::vector<Part>& parts);
};
