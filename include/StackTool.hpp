#pragma once

#include <string>
#include <vector>

class ImageSequence;
class Image;
class OffsetsFile;

class StackTool
{
public:
    StackTool();

    bool run(int argc, char* argv[]);
    void printUsage();

private:
    bool parseArgs(int argc, char* argv[]);

    void runQualityThread(int t, int tCount, const ImageSequence& imgSeq, std::vector<float>& quality);
    void runSumThread(int t, int tCount, const ImageSequence& imgSeq, const Image& dark, const OffsetsFile& offsets, int minX, int minY, Image& sum, const std::vector<float>& quality, float qualityMin);

    // Values read from cmd-line args
    std::string m_inPath;
    std::string m_inDarkFrameFn;
    std::string m_inOffsetsFn;
    std::string m_outFn;
    float m_minQualityPerc;
};
