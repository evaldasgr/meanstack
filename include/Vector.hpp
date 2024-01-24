#pragma once

#include <string>

struct Vector
{
    int x = 0;
    int y = 0;

    bool parse(const std::string& str);
};
