#pragma once

#include <string>
#include <Vector.hpp>

class AlignTool
{
public:
    AlignTool();

    bool run(int argc, char* argv[]);
    void printUsage();

private:
    bool parseArgs(int argc, char* argv[]);

    // Values read from cmd-line args
    std::string m_inPath;
    std::string m_inOffsetsFn;
    Vector m_objectPos;
    int m_searchRadius;
    int m_offsetRadius;
    std::string m_outFn;
};
