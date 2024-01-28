#include <AlignTool.hpp>
#include <iostream>
#include <cstring>
#include <fstream>
#include <cmath>
#include <ArgParser.hpp>
#include <ImageSequence.hpp>
#include <Image.hpp>
#include <OffsetsFile.hpp>

AlignTool::AlignTool():
    m_searchRadius(0),
    m_offsetRadius(0)
{

}

bool AlignTool::run(int argc, char* argv[])
{
    if (!parseArgs(argc, argv))
        return false;

    ImageSequence imgSeq;
    if (!imgSeq.open(m_inPath))
        return false;

    Image refImg;
    if (!refImg.load(imgSeq.getFilename(0)))
        return false;

    OffsetsFile offsets;
    if (m_inOffsetsFn.empty())
        offsets.create(imgSeq.getCount());
    else
    {
        if (!offsets.load(m_inOffsetsFn))
            return false;
        if (offsets.getCount() != imgSeq.getCount())
        {
            std::cerr << "Error: Offsets file index count does not match the image count" << std::endl;
            return false;
        }
    }

    Vector startOffset = offsets.get(0);
    int progress = 0;
    #pragma omp parallel for
    for (int i = 0; i < imgSeq.getCount(); i++)
    {
        #pragma omp critical
        {
            progress++;
            if (progress % 100 == 0)
                std::cout << "Processing image " << progress << "/" << imgSeq.getCount() << "..." << std::endl;
        }

        Image img;
        if (!img.load(imgSeq.getFilename(i)))
            continue;

        Vector offset = offsets.get(i);

        // Local object position after applied offset in this image coordinates
        Vector localObjectPos = {
            m_objectPos.x + startOffset.x - offset.x,
            m_objectPos.y + startOffset.y - offset.y
        };

        // Is the search radius within bounds of the image?
        if (localObjectPos.x - m_searchRadius < 0 || localObjectPos.y - m_searchRadius < 0 || localObjectPos.x + m_searchRadius >= img.getWidth() || localObjectPos.y + m_searchRadius >= img.getHeight())
        {
            std::cerr << "Error: Out of bounds search area in image " << i << std::endl;
            continue;
        }

        Vector bestOffset;
        float bestDif = INFINITY;
        for (int oy = -m_offsetRadius; oy <= m_offsetRadius; oy++)
        {
            for (int ox = -m_offsetRadius; ox <= m_offsetRadius; ox++)
            {
                float dif = 0;
                // Make sure some signal was measured to avoid best matches on
                // black background
                bool valid = false;
                for (int sy = -m_searchRadius; sy <= m_searchRadius; sy++)
                {
                    for (int sx = -m_searchRadius; sx <= m_searchRadius; sx++)
                    {
                        int imgX = localObjectPos.x + sx - ox;
                        int imgY = localObjectPos.y + sy - oy;
                        float imgVal = img.getSample(imgX, imgY, 0) + img.getSample(imgX, imgY, 1) + img.getSample(imgX, imgY, 2);

                        int refX = m_objectPos.x + sx;
                        int refY = m_objectPos.y + sy;
                        float refVal = refImg.getSample(refX, refY, 0) + refImg.getSample(refX, refY, 1) + refImg.getSample(refX, refY, 2);

                        dif += std::abs(imgVal - refVal);

                        // Hardcoded signal strength of 0.5 (multiplied by 3 as
                        // imgVal is sum of 3 channels)
                        if (imgVal >= 0.5f * 3.f)
                            valid = true;
                    }
                }

                if (valid && dif < bestDif)
                {
                    bestOffset = { ox, oy };
                    bestDif = dif;
                }
            }
        }
        if (bestDif == INFINITY)
        {
            std::cerr << "Error: Failed to find best offset in image " << i << std::endl;
            continue;
        }

        offsets.set(i, { offset.x + bestOffset.x , offset.y + bestOffset.y });
    }

    if (!offsets.save(m_outFn))
        return false;

    std::cout << "Done" << std::endl;

    return true;
}

void AlignTool::printUsage()
{
    std::cerr << "Usage: meanstack align -i dir [-O file] -p x,y -s num -m num -o file" << std::endl
        << "-i dir  Input image sequence directory" << std::endl
        << "-O file Input offsets file, optional" << std::endl
        << "-p x,y  Position of object in first image" << std::endl
        << "-s num  Search radius" << std::endl
        << "-m num  Maximum offset radius" << std::endl
        << "-o file Output offsets file" << std::endl;
}

bool AlignTool::parseArgs(int argc, char* argv[])
{
    ArgParser parser;
    parser.registerString("-i", true);
    parser.registerString("-O");
    parser.registerVector("-p", true);
    parser.registerInt("-s", true);
    parser.registerInt("-m", true);
    parser.registerString("-o", true);

    if (!parser.parse(argc, argv, 2))
        return false;

    m_inPath = parser.getString("-i");
    if (parser.hasString("-O"))
        m_inOffsetsFn = parser.getString("-O");
    m_objectPos = parser.getVector("-p");
    m_searchRadius = parser.getInt("-s");
    m_offsetRadius = parser.getInt("-m");
    m_outFn = parser.getString("-o");

    return true;
}
