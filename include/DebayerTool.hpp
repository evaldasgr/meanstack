#pragma once

#include <string>

class DebayerTool
{
public:
    bool run(int argc, char* argv[]);
    void printUsage();

private:
    bool parseArgs(int argc, char* argv[]);

    // Values read from cmd-line args
    std::string m_inPath;
    std::string m_outFormat;
    std::string m_outPath;
};
