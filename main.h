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
    std::string fileIn;

    commandInformation(lyra::cli &cli);

    int doCommand(const lyra::group &g);
};

struct fileInfo {
    u_int64_t fileSize;
};

uint32_t readFile(const char *filename, uint8_t **data_out);

int combineStreams(const char *fileL, const char *fileH, const char *output_file, bool little_endian, uint8_t word_size);

int collectFileInfo(const char *fileIn);

#endif //MAIN_H
