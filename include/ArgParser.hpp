#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <Vector.hpp>

class ArgParser
{
public:
    void registerInt(const std::string& name, bool required = false);
    void registerFloat(const std::string& name, bool required = false);
    void registerString(const std::string& name, bool required = false);
    void registerVector(const std::string& name, bool required = false);
    void registerFlag(const std::string& name);

    bool parse(int argc, char* argv[], int startFrom = 1);

    bool hasInt(const std::string& name) const;
    bool hasFloat(const std::string& name) const;
    bool hasString(const std::string& name) const;
    bool hasVector(const std::string& name) const;
    bool hasFlag(const std::string& name) const;

    int getInt(const std::string& name) const;
    float getFloat(const std::string& name) const;
    const std::string& getString(const std::string& name) const;
    const Vector& getVector(const std::string& name) const;

private:
    enum class ArgType
    {
        Int, Float, String, Vector, Flag
    };

    struct ArgProps
    {
        ArgType type;
        bool required;
    };

    std::unordered_map<std::string, ArgProps> m_props;

    std::unordered_map<std::string, int> m_ints;
    std::unordered_map<std::string, float> m_floats;
    std::unordered_map<std::string, std::string> m_strings;
    std::unordered_map<std::string, Vector> m_vectors;
    std::unordered_set<std::string> m_flags;
};
