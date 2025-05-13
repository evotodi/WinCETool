#include "main.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>
#include "spdlog/spdlog.h"
#include <lyra/lyra.hpp>
#include <locale>
#include <filesystem>

using namespace std;

/* CLI Command: Interleave */
commandInterleave::commandInterleave(lyra::cli &cli) {
    cli.add_argument(
        lyra::command("interleave",
                      [this](const lyra::group &g) {
                          this->doCommand(g);
                      })
        .help("Combine two files into one by interleaving bytes from both files")
        .add_argument(lyra::help(showHelp))
        .add_argument(lyra::opt(verbose).name("-v").name("--verbose").optional().help("Verbose output"))
        .add_argument(lyra::opt(bigEndian).name("-b").name("--big-endian").help("Use big endian. Same a swapping fileL and fileH."))
        .add_argument(lyra::opt(word, "size").name("-w").name("--word").help("Word size to build. 16, 32, 64"))
        .add_argument(lyra::arg(fileL, "file low").required().help("File Low Path"))
        .add_argument(lyra::arg(fileH, "file high").required().help("File High Path"))
        .add_argument(lyra::arg(output, "output file").required().help("Output File Path"))
    );
}

int commandInterleave::doCommand(const lyra::group &g) {
    if (showHelp) {
        std::cout << g;

        return 0;
    }

    littleEndian = !bigEndian;

    if (verbose) {
        spdlog::set_level(spdlog::level::debug);
    }

    spdlog::debug("File Low Path = {}", fileL);
    spdlog::debug("File High Path = {}", fileH);
    spdlog::debug("Output File Path = {}", output);
    spdlog::debug("Use Little Endian = {}", (littleEndian ? "true" : "false"));

    return combineStreams(fileL.c_str(), fileH.c_str(), output.c_str(), littleEndian, word);
}

/* CLI Command: Info */

commandInformation::commandInformation(lyra::cli &cli) {
    cli.add_argument(
        lyra::command("info",
                      [this](const lyra::group &g) {
                          this->doCommand(g);
                      })
        .help("Get information about the file")
        .add_argument(lyra::opt(verbose).name("-v").name("--verbose").optional().help("Verbose output"))
        .add_argument(lyra::opt(baseAddressStr, "hex string").name("-b").name("--base").optional().help("Base Address in hex"))
        .add_argument(lyra::opt(bigEndian).name("-b").name("--big-endian").help("Use big endian"))
        .add_argument(lyra::arg(fileIn, "file").required().help("Input File Path"))
    );
};

int commandInformation::doCommand(const lyra::group &g) {
    if (showHelp) {
        std::cout << g;
        return 0;
    }

    littleEndian = !bigEndian;

    if (verbose) {
        spdlog::set_level(spdlog::level::debug);
    }

    return collectFileInfo(fileIn.c_str(), baseAddressStr, littleEndian);
}

/* Helper Functions */

long hexStringToLong(const std::string& hexString) {
    try {
        return std::stol(hexString, nullptr, 16);
    } catch (const std::invalid_argument& ia) {
        spdlog::error("Invalid argument: {}", ia.what());
        exit(1);
    } catch (const std::out_of_range& oor) {
        spdlog::error("Out of range error: ", oor.what());
        exit(1);
    }
}

/* File Operations */

uint32_t readFile(const char *filename, uint8_t **data_out) {
    FILE *file = fopen(filename, "rb");
    fseek(file, 0, SEEK_END);
    uint32_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t *data = (uint8_t *) malloc(size);
    for (uint32_t i = 0; i < size; ++i)
        fread(&data[i], 1, 1, file);
    fclose(file);
    *data_out = data;
    return size;
}

int combineStreams(const char *fileL, const char *fileH, const char *output_file, bool littleEndian, uint8_t wordSize) {
    if (wordSize != 16 && wordSize != 32 && wordSize != 64) {
        spdlog::error("Error: Invalid word size. Exiting!");
        return 1; // invalid word size
    }

    uint8_t bytesPerFile;
    if (wordSize == 16) {
        spdlog::debug("Word size = 16");
        bytesPerFile = 1;
    } else if (wordSize == 32) {
        spdlog::debug("Word size = 32");
        bytesPerFile = 2;
    } else {
        spdlog::debug("Word size = 64");
        bytesPerFile = 4;
    }

    uint8_t *data[2];
    uint32_t fsize[2];
    spdlog::debug("Reading file {}", fileL);
    fsize[0] = readFile(fileL, &data[0]);
    spdlog::info("File Low Size = {:L}", fsize[0]);
    spdlog::debug("Reading file {}", fileH);
    fsize[1] = readFile(fileH, &data[1]);
    spdlog::info("File High Size = {:L}", fsize[1]);

    if (fsize[0] != fsize[1]) {
        spdlog::error("Error: File sizes are not the same. Exiting!");
        return 2; // file sizes are mismatching
    }

    uint32_t size = fsize[0];
    spdlog::info("Output file size = {:L}", fsize[0] + fsize[1]);

    // Set endianness
    uint8_t *dataL = (littleEndian ? data[0] : data[1]);
    uint8_t *dataH = (littleEndian ? data[1] : data[0]);

    FILE *out_file = fopen(output_file, "wb");
    uint8_t data_out;
    for (uint32_t i = 0; i < size; i += bytesPerFile) {
        for (uint8_t j = 0; j < bytesPerFile; ++j) {
            data_out = dataH[i + j];
            fwrite(&data_out, 1, 1, out_file);
        }

        for (uint8_t j = 0; j < bytesPerFile; ++j) {
            data_out = dataL[i + j];
            fwrite(&data_out, 1, 1, out_file);
        }
    }

    fclose(out_file);

    free(data[0]);
    free(data[1]);
    spdlog::info("Done!");
    return 0;
}

int collectFileInfo(const char *fileIn, string baseAddressStr, bool littleEndian) {
    spdlog::info("Using file {}", fileIn);
    spdlog::debug("Using base address: {}", baseAddressStr);
    spdlog::debug("Use Little Endian = {}", (littleEndian ? "true" : "false"));

    // Convert base address string to long
    long baseAddress = hexStringToLong(baseAddressStr);
    spdlog::debug("Base Address dec: {} hex: {}", baseAddress, baseAddressStr);

    filesystem::path path(fileIn);
    spdlog::info("File Size = {:L}", filesystem::file_size(path));

    ifstream file(path.string(), ios::binary);
    if (!file.is_open()) {
        spdlog::error("Error: Could not open file. Exiting!");
        return 1; // file not found
    }

    spdlog::info("Reading in file...");

    // Read the entire file into a string
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    spdlog::info("Processing file...");

    // Pattern BO0OFF\n
    regex patternBoof("\\x42\\x30\\x30\\x30\\x46\\x46\\x0A");

    // Search for the B000FF pattern
    smatch match;
    if (regex_search(content, match, patternBoof)) {
        spdlog::info("Found SYNC bytes B000FF");
        spdlog::debug("Found at offset: dec: {} hex: {}", match.position(), fmt::format("{:#x}", match.position()));
        spdlog::debug("Found length: {}", match.length());
    } else {
        spdlog::info("SYNC pattern not found. Exiting");
        exit(1);
    }

    stringstream tempHexStr;

    // Get the Least address
    string leastAddress = content.substr(match.position() + match.length(), 4);
    tempHexStr.str("0x"); // Reset stream
    tempHexStr.clear(); // Clear error flags
    if (littleEndian) {
        for (int i = leastAddress.length() - 1; i >= 0 ; i--) {
            tempHexStr << fmt::format("{:02x}", leastAddress[i]);
        }
    } else {
        for (int i = 0; i < leastAddress.length() ; i++) {
            tempHexStr << fmt::format("{:02x}", leastAddress[i]);
        }
    }

    long leastAddressLong = hexStringToLong(tempHexStr.str());
    spdlog::debug("Least Address Real dec: {} hex: {:#08x}", leastAddressLong, leastAddressLong);
    leastAddressLong = leastAddressLong - baseAddress;
    spdlog::debug("Least Address dec: {} hex: {:#08x}", leastAddressLong, leastAddressLong);


    // Get the Greatest address
    string greatestAddress = content.substr(match.position() + match.length() + 4, 4);
    tempHexStr.str("0x"); // Reset stream
    tempHexStr.clear(); // Clear error flags
    if (littleEndian) {
        for (int i = greatestAddress.length() - 1; i >= 0 ; i--) {
            tempHexStr << fmt::format("{:02x}", greatestAddress[i]);
        }
    } else {
        for (int i = 0; i < greatestAddress.length() ; i++) {
            tempHexStr << fmt::format("{:02x}", greatestAddress[i]);
        }
    }

    long greatestAddressLong = hexStringToLong(tempHexStr.str());
    spdlog::debug("Greatest Address dec: {} hex: {:#02x}", greatestAddressLong, greatestAddressLong);

    // Calculate end address
    long endAddress = greatestAddressLong - leastAddressLong;
    spdlog::debug("End Address dec: {} hex: {}", endAddress, fmt::format("{:#x}", endAddress));


    return 0;
}

/* Main */

int main(int argc, char *argv[]) {
    std::locale::global(std::locale("en_US.UTF-8"));
    spdlog::set_pattern("%v");
    spdlog::set_level(spdlog::level::info);


    auto cli = lyra::cli();
    bool showHelp = false;


    cli.add_argument(lyra::help(showHelp).description("Win CE Flash Tool"));
    commandInterleave cmdInterleave{cli};
    commandInformation cmdInfo{cli};
    auto result = cli.parse({argc, argv});

    if (showHelp || argc == 1) {
        std::cout << cli << endl;

        return 0;
    }

    if (!result) {
        std::cout << cli << endl;
        std::cerr << "Error in command line: " << result.message() << std::endl;
    }

    return result ? 0 : 1;
}
