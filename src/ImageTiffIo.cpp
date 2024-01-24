#include <ImageTiffIo.hpp>
#include <iostream>
#include <tiffio.h>
#include <Image.hpp>

bool ImageTiffIo::load(const std::string& filename, Image& image)
{
    TIFF* in = TIFFOpen(filename.c_str(), "r");
    if (!in)
        return false;

    // TODO: Add support for 8-bit and 16-bit images
    uint16_t bitDepth;
    TIFFGetField(in, TIFFTAG_BITSPERSAMPLE, &bitDepth);
    if (bitDepth != 32)
    {
        std::cerr << "Error: The TIFF must be 32-bit" << std::endl;
        return false;
    }

    uint16_t samples, format;
    TIFFGetField(in, TIFFTAG_SAMPLESPERPIXEL, &samples);
    TIFFGetField(in, TIFFTAG_SAMPLEFORMAT, &format);
    if (format != SAMPLEFORMAT_IEEEFP)
    {
        std::cerr << "Error: The TIFF must be floating point" << std::endl;
        return false;
    }

    uint32_t width = 0, height = 0;
    TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(in, TIFFTAG_IMAGELENGTH, &height);

    image.create(width, height, samples);

    std::vector<float> data(width * samples);
    for (int y = 0; y < height; y++)
    {
        if (TIFFReadScanline(in, &data[0], y) == -1)
        {
            TIFFClose(in);
            std::cerr << "Error: Could not read the image data" << std::endl;
            return false;
        }

        for (int x = 0; x < width; x++)
        {
            for (int c = 0; c < samples; c++)
                image.setSample(x, y, c, data[x * samples + c]);
        }
    }

    TIFFClose(in);

    return true;
}

bool ImageTiffIo::save(const std::string& filename, const Image& image)
{
    TIFF* out = TIFFOpen(filename.c_str(), "w");
    if (!out)
    {
        std::cerr << "Error: Failed to open TIFF file for writing" << std::endl;
        return false;
    }

    if (!TIFFSetField(out, TIFFTAG_IMAGEWIDTH, image.getWidth()) ||
        !TIFFSetField(out, TIFFTAG_IMAGELENGTH, image.getHeight()) ||
        !TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 32) ||
        !TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB) ||
        !TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, image.getChannels()) ||
        !TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG) ||
        !TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP))
    {
        std::cerr << "Error: Failed to write the TIFF header" << std::endl;
        TIFFClose(out);
        return false;
    }

    if (image.getChannels() == 4)
    {
        constexpr short extraSamples[] = { EXTRASAMPLE_UNASSALPHA };
        if (!TIFFSetField(out, TIFFTAG_EXTRASAMPLES, 1, extraSamples))
        {
            std::cerr << "Error: Failed to set the TIFF extra samples field" << std::endl;
            TIFFClose(out);
            return false;
        }
    }

    std::vector<float> data(image.getWidth() * image.getChannels());
    for (int y = 0; y < image.getHeight(); y++)
    {
        for (int x = 0; x < image.getWidth(); x++)
        {
            for (int c = 0; c < image.getChannels(); c++)
                data[x * image.getChannels() + c] = image.getSample(x, y, c);
        }

        if (TIFFWriteScanline(out, &data[0], y, 0) != 1)
        {
            std::cerr << "Error: Failed to write the image data" << std::endl;
            TIFFClose(out);
            return false;
        }
    }

    TIFFClose(out);

    return true;
}
