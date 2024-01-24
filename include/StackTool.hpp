#pragma once

#include <string>

class ImageSequence;
class Image;
class OffsetsFile;

class StackTool
{
public:
    bool run(int argc, char* argv[]);
    void printUsage();

private:
    bool parseArgs(int argc, char* argv[]);

    void runSumThread(int t, int tCount, const ImageSequence& imgSeq, const Image& dark, const OffsetsFile& offsets, int minX, int minY, Image& sum);

    // Values read from cmd-line args
    std::string m_inPath;
    std::string m_inDarkFrameFn;
    std::string m_inOffsetsFn;
    std::string m_outFn;
};
