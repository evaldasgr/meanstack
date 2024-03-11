#include <ImageJpegXlIo.hpp>
#include <iostream>
#include <fstream>
#include <jxl/decode_cxx.h>
#include <jxl/resizable_parallel_runner_cxx.h>
#include <jxl/encode_cxx.h>
#include <jxl/thread_parallel_runner_cxx.h>
#include <Image.hpp>

bool ImageJpegXlIo::load(const std::string& filename, Image& image)
{
    std::vector<char> fileData;
    if (!loadFile(filename, fileData))
        return false;

    auto dec = JxlDecoderMake(nullptr);
    if (JxlDecoderSubscribeEvents(dec.get(), JXL_DEC_BASIC_INFO | JXL_DEC_FULL_IMAGE) != JXL_DEC_SUCCESS)
    {
        std::cerr << "Error: JxlDecoderSubscribeEvents failed" << std::endl;
        return false;
    }
    auto runner = JxlResizableParallelRunnerMake(nullptr);
    if (JxlDecoderSetParallelRunner(dec.get(), JxlResizableParallelRunner, runner.get()) != JXL_DEC_SUCCESS)
    {
        std::cerr << "Error: JxlDecoderSetParallelRunner failed" << std::endl;
        return false;
    }

    JxlBasicInfo info;
    JxlPixelFormat format = { 4, JXL_TYPE_FLOAT, JXL_NATIVE_ENDIAN, 0 };

    JxlDecoderSetInput(dec.get(), (uint8_t*)&fileData[0], fileData.size());
    JxlDecoderCloseInput(dec.get());

    std::vector<float> data;
    JxlDecoderStatus status;
    do
    {
        status = JxlDecoderProcessInput(dec.get());

        if (status == JXL_DEC_ERROR || status == JXL_DEC_NEED_MORE_INPUT)
        {
            std::cerr << "Error: JxlDecoderProcessInput failed" << std::endl;
            return false;
        }
        else if (status == JXL_DEC_BASIC_INFO)
        {
            if (JxlDecoderGetBasicInfo(dec.get(), &info) != JXL_DEC_SUCCESS)
            {
                std::cerr << "Error: JxlDecoderGetBasicInfo failed" << std::endl;
                return false;
            }
            JxlResizableParallelRunnerSetThreads(runner.get(), JxlResizableParallelRunnerSuggestThreads(info.xsize, info.ysize));
        }
        else if (status == JXL_DEC_NEED_IMAGE_OUT_BUFFER)
        {
            std::size_t bufferSize;
            if (JxlDecoderImageOutBufferSize(dec.get(), &format, &bufferSize) != JXL_DEC_SUCCESS)
            {
                std::cerr << "Error: JxlDecoderImageOutBufferSize failed" << std::endl;
                return false;
            }
            if (bufferSize != info.xsize * info.ysize * 4 * sizeof(float))
            {
                std::cerr << "Error: Unexpected buffer size" << std::endl;
                return false;
            }
            data.resize(bufferSize);
            if (JxlDecoderSetImageOutBuffer(dec.get(), &format, &data[0], bufferSize) != JXL_DEC_SUCCESS)
            {
                std::cerr << "Error: JxlDecoderSetImageOutBuffer failed" << std::endl;
                return false;
            }
        }
    } while (status != JXL_DEC_SUCCESS);

    image.create(info.xsize, info.ysize);
    for (int y = 0; y < info.ysize; y++)
    {
        for (int x = 0; x < info.xsize; x++)
        {
            image.setSample(x, y, 0, data[(x + y * info.xsize) * 4 + 0]);
            image.setSample(x, y, 1, data[(x + y * info.xsize) * 4 + 1]);
            image.setSample(x, y, 2, data[(x + y * info.xsize) * 4 + 2]);
            image.setSample(x, y, 3, data[(x + y * info.xsize) * 4 + 3]);
        }
    }

    return true;
}

bool ImageJpegXlIo::save(const std::string& filename, const Image& image)
{
    auto enc = JxlEncoderMake(nullptr);
    auto runner = JxlThreadParallelRunnerMake(nullptr, JxlThreadParallelRunnerDefaultNumWorkerThreads());
    if (JxlEncoderSetParallelRunner(enc.get(), JxlThreadParallelRunner, runner.get()) != JXL_ENC_SUCCESS)
    {
        std::cerr << "Error: JxlEncoderSetParallelRunner failed" << std::endl;
        return false;
    }

    JxlPixelFormat pixel_format = { 4, JXL_TYPE_FLOAT, JXL_NATIVE_ENDIAN, 0 };

    JxlBasicInfo basic_info;
    JxlEncoderInitBasicInfo(&basic_info);
    basic_info.xsize = image.getWidth();
    basic_info.ysize = image.getHeight();
    basic_info.bits_per_sample = 32;
    basic_info.exponent_bits_per_sample = 8;
    basic_info.uses_original_profile = JXL_TRUE;
    basic_info.num_extra_channels = 1;
    basic_info.alpha_bits = 32;
    basic_info.alpha_exponent_bits = 8;
    basic_info.alpha_premultiplied = JXL_FALSE;
    if (JxlEncoderSetBasicInfo(enc.get(), &basic_info) != JXL_ENC_SUCCESS)
    {
        std::cerr << "Error: JxlEncoderSetBasicInfo failed" << std::endl;
        return false;
    }

    JxlColorEncoding color_encoding;
    JxlColorEncodingSetToSRGB(&color_encoding, JXL_FALSE);
    if (JxlEncoderSetColorEncoding(enc.get(), &color_encoding) != JXL_ENC_SUCCESS)
    {
        std::cerr << "Error: JxlEncoderSetColorEncoding failed" << std::endl;
        return false;
    }

    JxlEncoderFrameSettings* frame_settings = JxlEncoderFrameSettingsCreate(enc.get(), nullptr);
    if (JxlEncoderSetFrameLossless(frame_settings, JXL_TRUE) != JXL_ENC_SUCCESS)
    {
        std::cerr << "Error: JxlEncoderSetFrameLossless failed" << std::endl;
        return false;
    }

    std::vector<float> data;
    data.reserve(image.getWidth() * image.getHeight() * 4);
    for (int y = 0; y < image.getHeight(); y++)
    {
        for (int x = 0; x < image.getWidth(); x++)
        {
            data.push_back(image.getSample(x, y, 0));
            data.push_back(image.getSample(x, y, 1));
            data.push_back(image.getSample(x, y, 2));
            data.push_back(image.getSample(x, y, 3));
        }
    }

    if (JxlEncoderAddImageFrame(frame_settings, &pixel_format, &data[0], data.size() * sizeof(float)) != JXL_ENC_SUCCESS)
    {
        std::cerr << "Error: JxlEncoderAddImageFrame failed" << std::endl;
        return false;
    }
    JxlEncoderCloseInput(enc.get());

    std::vector<char> fileData(32 * 1024 * 1024);
    uint8_t* ptr = (uint8_t*)&fileData[0];
    auto avail = fileData.size();
    JxlEncoderStatus status;
    while (true)
    {
        status = JxlEncoderProcessOutput(enc.get(), &ptr, &avail);
        if (status == JXL_ENC_NEED_MORE_OUTPUT)
        {
            std::size_t written = ptr - (uint8_t*)&fileData[0];
            fileData.resize(fileData.size() * 2);
            ptr = (uint8_t*)&fileData[written];
            avail = fileData.size() - written;
        }
        else
            break;
    }
    fileData.resize(fileData.size() - avail);
    if (status != JXL_ENC_SUCCESS)
    {
        std::cerr << "Error: JxlEncoderProcessOutput failed" << std::endl;
        return false;
    }

    if (!saveFile(filename, fileData))
        return false;

    return true;
}

bool ImageJpegXlIo::loadFile(const std::string& filename, std::vector<char>& data)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
        return false;

    file.seekg(0, std::ios::end);
    auto size = file.tellg();
    data.resize(size);

    file.seekg(0, std::ios::beg);
    file.read(&data[0], size);

    return true;
}

bool ImageJpegXlIo::saveFile(const std::string& filename, const std::vector<char>& data)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open())
        return false;

    file.write(&data[0], data.size());

    return true;
}
