#include <Vector.hpp>

bool Vector::parse(const std::string& str)
{
    // Find comma seperator
    auto pos = str.find(',');
    if (pos == std::string::npos)
        return false;

    // No number after comma
    if (pos + 1 >= str.size())
        return false;

    std::string a = str.substr(0, pos);
    std::string b = str.substr(pos + 1);

    try
    {
        x = std::stoi(a);
        y = std::stoi(b);
    }
    catch (std::exception&)
    {
        return false;
    }

    return true;
}
