#include <cstring>
#include <iostream>
#include <chrono>
#include <AlignTool.hpp>
#include <StackTool.hpp>
#include <DebayerTool.hpp>

void printUsage();

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printUsage();
        return 1;
    }

    auto start = std::chrono::steady_clock::now();

    int ret;
    if (std::strcmp(argv[1], "align") == 0)
    {
        AlignTool tool;
        if (argc == 2)
        {
            tool.printUsage();
            return 1;
        }
        ret = tool.run(argc, argv) ? 1 : 0;
    }
    else if (std::strcmp(argv[1], "stack") == 0)
    {
        StackTool tool;
        if (argc == 2)
        {
            tool.printUsage();
            return 1;
        }
        ret = tool.run(argc, argv) ? 1 : 0;
    }
    else if (std::strcmp(argv[1], "debayer") == 0)
    {
        DebayerTool tool;
        if (argc == 2)
        {
            tool.printUsage();
            return 1;
        }
        ret = tool.run(argc, argv) ? 1 : 0;
    }
    else
    {
        printUsage();
        return 1;
    }

    std::chrono::duration<float> elapsed = std::chrono::steady_clock::now() - start;
    std::cout << "Elapsed time (s): " << elapsed.count() << std::endl;

    return ret;
}

void printUsage()
{
    std::cerr << "Usage: meanstack align/stack/debayer" << std::endl;
}
