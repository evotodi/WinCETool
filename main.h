#ifndef MAIN_H
#define MAIN_H

#include <lyra/lyra.hpp>

struct commandInterleave {
    bool showHelp = false;
    bool verbose = false;
    std::string fileL;
    std::string fileH;
    std::string output;
    bool littleEndian = true;
    bool bigEndian = false;
    int word = 16;

    commandInterleave(lyra::cli &cli);

    int doCommand(const lyra::group &g);
};

struct commandInformation {
    bool showHelp = false;
    bool verbose = false;
    bool littleEndian = true;
    bool bigEndian = false;
    std::string baseAddressStr = "0x0";
    std::string fileIn;

    commandInformation(lyra::cli &cli);

    int doCommand(const lyra::group &g);
};

struct fileInfo {
    uint64_t fileSize;
};

uint32_t readFile(const char *filename, uint8_t **data_out);

int combineStreams(const char *fileL, const char *fileH, const char *output_file, bool littleEndian, uint8_t wordSize);

int collectFileInfo(const char *fileIn, std::string baseAddressStr, bool littleEndian);

long hexStringToLong(const std::string& hexString);

#endif //MAIN_H
