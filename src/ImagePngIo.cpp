#include <ImagePngIo.hpp>
#include <iostream>
#include <algorithm>
#include <bit>
#include <png.h>
#include <Image.hpp>

bool ImagePngIo::load(const std::string& filename, Image& image)
{
    std::FILE* file = std::fopen(filename.c_str(), "rb");
    if (!file)
        return false;

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);

    png_init_io(png, file);

    png_read_info(png, info);

    int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    int colorType = png_get_color_type(png, info);
    int bitDepth = png_get_bit_depth(png, info);

    image.create(width, height);

    // Convert format to 16-bit RGBA
    if (bitDepth < 16)
        png_set_expand_16(png);

    if (colorType == PNG_COLOR_TYPE_RGB ||
        colorType == PNG_COLOR_TYPE_GRAY ||
        colorType == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if (colorType == PNG_COLOR_TYPE_GRAY ||
        colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    std::vector<unsigned char> data(png_get_rowbytes(png, info));
    for (int y = 0; y < height; y++)
    {
        png_read_row(png, &data[0], nullptr);
        for (int x = 0; x < width; x++)
        {
            for (int c = 0; c < 4; c++)
            {
                float sample = (float)((uint16_t*)(&data[0]))[x * 4 + c] / 65535.f;
                image.setSample(x, y, c, sample);
            }
        }
    }

    std::fclose(file);

    png_destroy_read_struct(&png, &info, NULL);

    return true;
}

bool ImagePngIo::save(const std::string& filename, const Image& image)
{
    std::FILE* file = std::fopen(filename.c_str(), "wb");
    if (!file)
        return false;

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);

    png_init_io(png, file);

    // PNG does not support 32-bit bit-depth, so reduce to 16-bit
    png_set_IHDR(png, info, image.getWidth(), image.getHeight(), 16, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    if constexpr (std::endian::native == std::endian::little)
        png_set_swap(png);

    std::vector<uint16_t> buf(image.getWidth() * 4);
    for (int y = 0; y < image.getHeight(); y++)
    {
        for (int x = 0; x < image.getWidth(); x++)
        {
            uint16_t* pixel = &buf[x * 4];

            pixel[0] = std::clamp((int)(image.getSample(x, y, 0) * 65535.f), 0, 65535);
            pixel[1] = std::clamp((int)(image.getSample(x, y, 1) * 65535.f), 0, 65535);
            pixel[2] = std::clamp((int)(image.getSample(x, y, 2) * 65535.f), 0, 65535);
            pixel[3] = std::clamp((int)(image.getSample(x, y, 3) * 65535.f), 0, 65535);
        }
        png_write_row(png, (png_const_bytep)&buf[0]);
    }

    png_write_end(png, nullptr);

    std::fclose(file);

    png_destroy_write_struct(&png, &info);

    return true;
}
