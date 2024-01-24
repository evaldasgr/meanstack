#include <StackTool.hpp>
#include <iostream>
#include <thread>
#include <ArgParser.hpp>
#include <ImageSequence.hpp>
#include <Image.hpp>
#include <OffsetsFile.hpp>
#include <Vector.hpp>

bool StackTool::run(int argc, char* argv[])
{
    if (!parseArgs(argc, argv))
        return false;

    ImageSequence imgSeq;
    if (!imgSeq.open(m_inPath))
        return false;

    Image darkImg;
    if (!m_inDarkFrameFn.empty())
    {
        if (!darkImg.load(m_inDarkFrameFn))
            return false;
    }

    OffsetsFile offsets;
    // If no offsets file provided, create a blank (zeroes) offsets object to
    // avoid having to check for existence of offsets every single time
    if (m_inOffsetsFn.empty())
        offsets.create(imgSeq.getCount());
    else
    {
        if (!offsets.load(m_inOffsetsFn))
            return false;
        if (offsets.getCount() != imgSeq.getCount())
        {
            std::cerr << "Error: Offsets file index count does not match image count" << std::endl;
            return false;
        }
    }

    // Determine size of output image
    Image refImg;
    if (!refImg.load(imgSeq.getFilename(0)))
        return false;

    int minX = offsets.findMinX();
    int minY = offsets.findMinY();
    int maxX = offsets.findMaxX() + refImg.getWidth();
    int maxY = offsets.findMaxY() + refImg.getHeight();

    int threadCount = std::thread::hardware_concurrency();

    // Create intermediate sum images for all threads
    std::vector<Image> threadSums(threadCount);
    for (Image& i : threadSums)
        i.create(maxX - minX, maxY - minY);

    // Run sum threads
    // TODO: Add progress indicator
    std::vector<std::thread> threads;
    for (int i = 0; i < threadCount; i++)
        threads.emplace_back(&StackTool::runSumThread, this, i, threadCount, std::cref(imgSeq), std::cref(darkImg), std::cref(offsets), minX, minY, std::ref(threadSums[i]));
    for (auto& t : threads)
        t.join();

    // Calculate the final sum
    Image outImg;
    outImg.create(maxX - minX, maxY - minY);
    for (const Image& i : threadSums)
    {
        for (int y = 0; y < outImg.getHeight(); y++)
        {
            for (int x = 0; x < outImg.getWidth(); x++)
            {
                outImg.setSample(x, y, 0, outImg.getSample(x, y, 0) + i.getSample(x, y, 0));
                outImg.setSample(x, y, 1, outImg.getSample(x, y, 1) + i.getSample(x, y, 1));
                outImg.setSample(x, y, 2, outImg.getSample(x, y, 2) + i.getSample(x, y, 2));
                outImg.setSample(x, y, 3, (float)((int)outImg.getSample(x, y, 3) + (int)i.getSample(x, y, 3)));
            }
        }
    }

    // Calculate the mean stack
    for (int y = 0; y < outImg.getHeight(); y++)
    {
        for (int x = 0; x < outImg.getWidth(); x++)
        {
            int samples = (int)outImg.getSample(x, y, 3);
            if (samples != 0)
            {
                outImg.setSample(x, y, 0, outImg.getSample(x, y, 0) / outImg.getSample(x, y, 3));
                outImg.setSample(x, y, 1, outImg.getSample(x, y, 1) / outImg.getSample(x, y, 3));
                outImg.setSample(x, y, 2, outImg.getSample(x, y, 2) / outImg.getSample(x, y, 3));
                outImg.setSample(x, y, 3, 1.f);
            }
        }
    }

    if (!outImg.save(m_outFn))
        return false;

    return true;
}

void StackTool::printUsage()
{
    std::cerr << "Usage: meanstack stack -i dir [-d file] [-O file] -o file" << std::endl
        << "-i dir  Input image sequence directory" << std::endl
        << "-d file Input dark frame image file, optional" << std::endl
        << "-O file Input offsets file, optional" << std::endl
        << "-o file Output image file" << std::endl;
}

bool StackTool::parseArgs(int argc, char* argv[])
{
    ArgParser parser;
    parser.registerString("-i", true);
    parser.registerString("-d");
    parser.registerString("-O");
    parser.registerString("-o", true);

    if (!parser.parse(argc, argv, 2))
        return false;

    m_inPath = parser.getString("-i");
    if (parser.hasString("-d"))
        m_inDarkFrameFn = parser.getString("-d");
    if (parser.hasString("-O"))
        m_inOffsetsFn = parser.getString("-O");
    m_outFn = parser.getString("-o");

    return true;
}

void StackTool::runSumThread(int t, int tCount, const ImageSequence& imgSeq, const Image& dark, const OffsetsFile& offsets, int minX, int minY, Image& sum)
{
    for (int i = t; i < imgSeq.getCount(); i++)
    {
        Image img;
        if (!img.load(imgSeq.getFilename(i)))
            continue;

        // Find corrected offset of this image, making sure that negative pixel
        // coordinates do not occur
        int offsetX = offsets.get(i).x - minX;
        int offsetY = offsets.get(i).y - minY;

        // Verify the image size
        if (img.getWidth() + offsetX > sum.getWidth() || img.getHeight() + offsetY > sum.getHeight())
        {
            std::cerr << "Error: Image " << i << " size mismatch" << std::endl;
            continue;
        }

        // If provided, apply the dark frame
        if (dark.getWidth() != 0)
        {
            if (dark.getWidth() != img.getWidth() || dark.getHeight() != img.getHeight())
            {
                std::cerr << "Error: Image " << i << " and dark frame sizes mismatch" << std::endl;
                continue;
            }

            for (int y = 0; y < img.getHeight(); y++)
            {
                for (int x = 0; x < img.getWidth(); x++)
                {
                    for (int c = 0; c < 3; c++)
                        img.setSample(x, y, c, img.getSample(x, y, c) - dark.getSample(x, y, c));
                }
            }
        }

        // Sum the image
        for (int y = 0; y < img.getHeight(); y++)
        {
            for (int x = 0; x < img.getWidth(); x++)
            {
                // If this pixel is transparent, do not stack it
                // This allows for source images that have some areas excluded
                // from the stack
                if (img.getChannels() == 4 && img.getSample(x, y, 3) < 0.5f)
                    continue;

                int sx = x + offsetX;
                int sy = y + offsetY;

                for (int c = 0; c < 3; c++)
                    sum.setSample(sx, sy, c, sum.getSample(sx, sy, c) + img.getSample(x, y, c));

                // For now the alpha channel is used as a sample counter
                // Cast to int and back is done to minimize floating point error
                // accumulation
                sum.setSample(sx, sy, 3, (float)((int)sum.getSample(sx, sy, 3) + 1));
            }
        }
    }
}
