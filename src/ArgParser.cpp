#include <ArgParser.hpp>
#include <iostream>
#include <cstring>
#include <Vector.hpp>

void ArgParser::registerInt(const std::string& name, bool required)
{
    ArgProps props;
    props.type = ArgType::Int;
    props.required = required;
    m_props[name] = props;
}

void ArgParser::registerString(const std::string& name, bool required)
{
    ArgProps props;
    props.type = ArgType::String;
    props.required = required;
    m_props[name] = props;
}

void ArgParser::registerVector(const std::string& name, bool required)
{
    ArgProps props;
    props.type = ArgType::Vector;
    props.required = required;
    m_props[name] = props;
}

void ArgParser::registerFlag(const std::string& name)
{
    ArgProps props;
    props.type = ArgType::Flag;
    m_props[name] = props;
}

bool ArgParser::parse(int argc, char* argv[], int startFrom)
{
    // Parse all arguments
    for (int i = startFrom; i < argc; i++)
    {
        for (const auto& p : m_props)
        {
            if (std::strcmp(argv[i], p.first.c_str()) != 0)
                continue;

            if (p.second.type == ArgType::Int)
            {
                if (++i == argc)
                {
                    std::cerr << "Error: Missing value for argument " << p.first << std::endl;
                    return false;
                }

                try
                {
                    int val = std::stoi(argv[i]);
                    m_ints[p.first] = val;
                }
                catch (std::exception&)
                {
                    std::cerr << "Error: Argument " << p.first << " is not a valid int" << std::endl;
                    return false;
                }
            }
            else if (p.second.type == ArgType::String)
            {
                if (++i == argc)
                {
                    std::cerr << "Error: Missing value for argument " << p.first << std::endl;
                    return false;
                }

                m_strings[p.first] = argv[i];
            }
            else if (p.second.type == ArgType::Vector)
            {
                if (++i == argc)
                {
                    std::cerr << "Error: Missing value for argument " << p.first << std::endl;
                    return false;
                }

                Vector vec;
                if (!vec.parse(argv[i]))
                {
                    std::cerr << "Error: Argument " << p.first << " is not a valid vector" << std::endl;
                    return false;
                }
                m_vectors[p.first] = vec;
            }
            else if (p.second.type == ArgType::Flag)
                m_flags.insert(p.first);

            break;
        }
    }

    // Make sure all required arguments were provided
    for (const auto& p : m_props)
    {
        if (!p.second.required)
            continue;

        switch (p.second.type)
        {
        case ArgType::Int:
            if (m_ints.find(p.first) == m_ints.end())
            {
                std::cerr << "Error: Required argument " << p.first << " was not specified" << std::endl;
                return false;
            }
            break;
        case ArgType::String:
            if (m_strings.find(p.first) == m_strings.end())
            {
                std::cerr << "Error: Required argument " << p.first << " was not specified" << std::endl;
                return false;
            }
            break;
        case ArgType::Vector:
            if (m_vectors.find(p.first) == m_vectors.end())
            {
                std::cerr << "Error: Required argument " << p.first << " was not specified" << std::endl;
                return false;
            }
            break;
        }
    }

    return true;
}

bool ArgParser::hasInt(const std::string& name) const
{
    return m_ints.find(name) != m_ints.end();
}

bool ArgParser::hasString(const std::string& name) const
{
    return m_strings.find(name) != m_strings.end();
}

bool ArgParser::hasVector(const std::string& name) const
{
    return m_vectors.find(name) != m_vectors.end();
}

bool ArgParser::hasFlag(const std::string& name) const
{
    return m_flags.find(name) != m_flags.end();
}

int ArgParser::getInt(const std::string& name) const
{
    return m_ints.at(name);
}

const std::string& ArgParser::getString(const std::string& name) const
{
    return m_strings.at(name);
}

const Vector& ArgParser::getVector(const std::string& name) const
{
    return m_vectors.at(name);
}
