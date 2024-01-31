#include <DebayerTool.hpp>
#include <iostream>
#include <numeric>
#include <algorithm>
#include <ArgParser.hpp>
#include <ImageSequence.hpp>
#include <Image.hpp>

bool DebayerTool::run(int argc, char* argv[])
{
    if (!parseArgs(argc, argv))
        return false;

    ImageSequence imgSeq;
    if (!imgSeq.open(m_inPath))
        return false;

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

        Image outImg;
        outImg.create(img.getWidth(), img.getHeight());

        // This algorithm debayers GRBG pattern Bayer images with linear
        // interpolation
        // Support for other patterns may be added in the future
        for (int y = 0; y < img.getHeight(); y++)
        {
            for (int x = 0; x < img.getWidth(); x++)
            {
                // Read existing values
                // Based on their position, valid values will be kept, and
                // invalid ones will be interpolated
                float r = img.getSample(x, y, 0);
                float g = img.getSample(x, y, 1);
                float b = img.getSample(x, y, 2);

                // Interpolate R if necessary
                if (x % 2 == 0 && y % 2 == 1)
                {
                    std::vector<float> v;
                    if (x > 0 && y > 0)
                        v.push_back(img.getSample(x - 1, y - 1, 0));
                    if (x + 1 < img.getWidth() && y > 0)
                        v.push_back(img.getSample(x + 1, y - 1, 0));
                    if (x > 0 && y + 1 < img.getHeight())
                        v.push_back(img.getSample(x - 1, y + 1, 0));
                    if (x + 1 < img.getWidth() && y + 1 < img.getHeight())
                        v.push_back(img.getSample(x + 1, y + 1, 0));
                    if (v.size() > 0)
                        r = std::accumulate(v.begin(), v.end(), 0.f) / v.size();
                }
                else if (x % 2 == 0)
                {
                    std::vector<float> v;
                    if (x > 0)
                        v.push_back(img.getSample(x - 1, y, 0));
                    if (x + 1 < img.getWidth())
                        v.push_back(img.getSample(x + 1, y, 0));
                    if (v.size() > 0)
                        r = std::accumulate(v.begin(), v.end(), 0.f) / v.size();
                }
                else if (y % 2 == 1)
                {
                    std::vector<float> v;
                    if (y > 0)
                        v.push_back(img.getSample(x, y - 1, 0));
                    if (y + 1 < img.getHeight())
                        v.push_back(img.getSample(x, y + 1, 0));
                    if (v.size() > 0)
                        r = std::accumulate(v.begin(), v.end(), 0.f) / v.size();
                }

                // Interpolate G if necessary
                if ((x % 2 == 1 && y % 2 == 0) || (x % 2 == 0 && y % 2 == 1))
                {
                    std::vector<float> v;
                    if (x > 0)
                        v.push_back(img.getSample(x - 1, y, 1));
                    if (x + 1 < img.getWidth())
                        v.push_back(img.getSample(x + 1, y, 1));
                    if (y > 0)
                        v.push_back(img.getSample(x, y - 1, 1));
                    if (y + 1 < img.getHeight())
                        v.push_back(img.getSample(x, y + 1, 1));
                    if (v.size() > 0)
                        g = std::accumulate(v.begin(), v.end(), 0.f) / v.size();
                }

                // Interpolate B if necessary
                if (x % 2 == 1 && y % 2 == 0)
                {
                    std::vector<float> v;
                    if (x > 0 && y > 0)
                        v.push_back(img.getSample(x - 1, y - 1, 2));
                    if (x + 1 < img.getWidth() && y > 0)
                        v.push_back(img.getSample(x + 1, y - 1, 2));
                    if (x > 0 && y + 1 < img.getHeight())
                        v.push_back(img.getSample(x - 1, y + 1, 2));
                    if (x + 1 < img.getWidth() && y + 1 < img.getHeight())
                        v.push_back(img.getSample(x + 1, y + 1, 2));
                    if (v.size() > 0)
                        b = std::accumulate(v.begin(), v.end(), 0.f) / v.size();
                }
                else if (x % 2 == 1)
                {
                    std::vector<float> v;
                    if (x > 0)
                        v.push_back(img.getSample(x - 1, y, 2));
                    if (x + 1 < img.getWidth())
                        v.push_back(img.getSample(x + 1, y, 2));
                    if (v.size() > 0)
                        b = std::accumulate(v.begin(), v.end(), 0.f) / v.size();
                }
                else if (y % 2 == 0)
                {
                    std::vector<float> v;
                    if (y > 0)
                        v.push_back(img.getSample(x, y - 1, 2));
                    if (y + 1 < img.getHeight())
                        v.push_back(img.getSample(x, y + 1, 2));
                    if (v.size() > 0)
                        b = std::accumulate(v.begin(), v.end(), 0.f) / v.size();
                }

                // Set values in the output image
                outImg.setSample(x, y, 0, r);
                outImg.setSample(x, y, 1, g);
                outImg.setSample(x, y, 2, b);
                outImg.setSample(x, y, 3, img.getSample(x, y, 3));
            }
        }

        outImg.save(m_outPath + "/" + std::to_string(i) + "." + m_outFormat);
    }

    std::cout << "Done" << std::endl;

    return true;
}

void DebayerTool::printUsage()
{
    std::cerr << "Usage: meanstack debayer -i dir [-f format] -o dir" << std::endl
        << "-i dir    Input image sequence directory" << std::endl
        << "-f format Output image format (TIFF/PNG), optional (default - TIFF)" << std::endl
        << "-o dir    Output image sequence directory" << std::endl;
}

bool DebayerTool::parseArgs(int argc, char* argv[])
{
    ArgParser parser;
    parser.registerString("-i", true);
    parser.registerString("-f");
    parser.registerString("-o", true);

    if (!parser.parse(argc, argv, 2))
        return false;

    m_inPath = parser.getString("-i");
    m_outPath = parser.getString("-o");

    if (parser.hasString("-f"))
    {
        m_outFormat = parser.getString("-f");
        std::transform(m_outFormat.begin(), m_outFormat.end(), m_outFormat.begin(), [](unsigned char c) { return std::tolower(c); });
        if (m_outFormat.compare("tiff") != 0 && m_outFormat.compare("tif") != 0 && m_outFormat.compare("png") != 0)
        {
            std::cerr << "Error: Unrecognized image format" << std::endl;
            return false;
        }
    }
    else
        m_outFormat = "tiff";

    return true;
}
