#include <FilenameComparator.hpp>

bool FilenameComparator::compare(const std::string& a, const std::string& b)
{
    std::vector<Part> aParts;
    split(a, aParts);

    std::vector<Part> bParts;
    split(b, bParts);

    int minParts = std::min(aParts.size(), bParts.size());
    for (int i = 0; i < minParts; i++)
    {
        if (aParts[i].digits && bParts[i].digits)
        {
            int comp = compareNum(aParts[i].str, bParts[i].str);
            if (comp < 0)
                return true;
            else if (comp > 0)
                return false;
        }
        else
        {
            int comp = aParts[i].str.compare(bParts[i].str);
            if (comp < 0)
                return true;
            else if (comp > 0)
                return false;
        }
    }

    return ((int)a.size() - (int)b.size()) < 0 ? true : false;
}

void FilenameComparator::split(const std::string& str, std::vector<Part>& parts)
{
    if (str.empty())
        return;

    parts.emplace_back();
    parts.back().digits = std::isdigit(str[0]);
    parts.back().str += str[0];
    for (int i = 1; i < str.size(); i++)
    {
        char c = str[i];

        if (std::isdigit(c) && !parts.back().digits)
        {
            parts.emplace_back();
            parts.back().digits = true;
        }
        else if (!std::isdigit(c) && parts.back().digits)
            parts.emplace_back();

        parts.back().str += c;
    }
}

int FilenameComparator::compareNum(const std::string& a, const std::string& b)
{
    if (a.size() < b.size())
        return -1;
    else if (a.size() > b.size())
        return 1;

    for (int i = 0; i < a.size(); i++)
    {
        if (a[i] < b[i])
            return -1;
        else if (a[i] > b[i])
            return 1;
    }

    return 0;
}
