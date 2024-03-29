#include <ImageJpegIo.hpp>
#include <iostream>
#include <jpeglib.h>
#include <Image.hpp>

bool ImageJpegIo::load(const std::string& filename, Image& image)
{
    struct jpeg_decompress_struct info;
    struct jpeg_error_mgr err;

    std::FILE* file;
    file = fopen(filename.c_str(), "rb");
    if (!file)
        return false;

    info.err = jpeg_std_error(&err);
    jpeg_create_decompress(&info);

    jpeg_stdio_src(&info, file);
    jpeg_read_header(&info, TRUE);

    jpeg_start_decompress(&info);

    image.create(info.output_width, info.output_height);

    std::vector<unsigned char> data(info.output_width * info.num_components);
    while (info.output_scanline < info.output_height)
    {
        // libjpeg requires an array of row pointers
        unsigned char* rowPtrs[1];
        rowPtrs[0] = (unsigned char*)(&data[0]);
        jpeg_read_scanlines(&info, rowPtrs, 1);

        for (int x = 0; x < info.output_width; x++)
        {
            int y = info.output_scanline - 1;
            for (int c = 0; c < info.num_components; c++)
            {
                float sample = (float)data[x * info.num_components + c] / 255.f;
                image.setSample(x, y, c, sample);
            }
            // In case of a grayscale image, duplicate values for G and B channels
            if (info.num_components < 3)
            {
                image.setSample(x, y, 1, image.getSample(x, y, 0));
                image.setSample(x, y, 2, image.getSample(x, y, 0));
            }
            // JPEGs don't support transparency, so always set alpha to opaque
            image.setSample(x, y, 3, 1.f);
        }
    }

    jpeg_finish_decompress(&info);
    jpeg_destroy_decompress(&info);
    std::fclose(file);

    return true;
}

bool ImageJpegIo::save(const std::string& filename, const Image& image)
{
    std::cerr << "Error: JPEG saving is not implemented yet" << std::endl;
    return false;
}
