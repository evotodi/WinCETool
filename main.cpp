#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <lyra/lyra.hpp>

using namespace std;

/**
 * @param filename path to file
 * @param data_out 
 * @return file size
 */
uint32_t read_file(const char *filename, uint8_t **data_out) {
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

/**
 * WordSize Examples:
 * word_size = 16: 8 bytes from file 1 and 8 bytes from file 2
 * word_size = 32: 16 bytes from file 1 and 16 bytes from file 2
 * word_size = 64: 32 bytes from file 1 and 32 bytes from file 2
 *
 * Little Endian true uses fileH and file 1
 *
 * @param fileL path to file low
 * @param fileH path to file high
 * @param output_file path to output file
 * @param little_endian use little endian encoding
 * @param word_size word size as 16, 32, or 64. Default 16.
 * @return 
 */
int combine_streams(const char *fileL, const char *fileH, const char *output_file, bool little_endian, uint8_t word_size = 16) {
    if (word_size != 16 && word_size != 32 && word_size != 64) {
        cout << "Error: Invalid word size. Exiting!" << endl;
        return 1; // invalid word size
    }

    uint8_t bytesPerFile;
    if (word_size == 16) {
        cout << "Word size = 16" << endl;
        bytesPerFile = 1;
    } else if (word_size == 32) {
        cout << "Word size = 32" << endl;
        bytesPerFile = 2;
    } else {
        cout << "Word size = 64" << endl;
        bytesPerFile = 4;
    }

    uint8_t *data[2];
    uint32_t fsize[2];
    cout << "Reading file " << fileL << endl;
    fsize[0] = read_file(fileL, &data[0]);
    cout << "File Low Size = " << fsize[0] << endl;
    cout << "Reading file " << fileH << endl;
    fsize[1] = read_file(fileH, &data[1]);
    cout << "File High Size = " << fsize[1] << endl;

    if (fsize[0] != fsize[1]) {
        cout << "Error: File sizes are not the same. Exiting!" << endl;
        return 2; // file sizes are mismatching
    }

    uint32_t size = fsize[0];
    cout << "Output file size = " << size << endl;

    // Set endianness
    uint8_t *dataL = (little_endian ? data[0] : data[1]);
    uint8_t *dataH = (little_endian ? data[1] : data[0]);

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
    return 0;
}

int main(int argc, char *argv[]) {
    std::string fileL;
    std::string fileH;
    std::string output;
    bool little_endian = true;
    bool big_endian = false;
    bool help = false;
    int word = 16;
    auto cli = lyra::cli();

    cli.add_argument(lyra::help(help).description("Combine two files into one by interleaving bytes from both files"));
    cli.add_argument(lyra::opt(big_endian).name("-b").name("--big-endian").help("Use big endian. Same a swapping fileL and fileH."));
    cli.add_argument(lyra::opt(word, "size").name("-w").name("--word").help("Word size to build. 16, 32, 64"));
    cli.add_argument(lyra::arg(fileL, "file low").required().help("File Low Path"));
    cli.add_argument(lyra::arg(fileH, "file high").required().help("File High Path"));
    cli.add_argument(lyra::arg(output, "output file").required().help("Output File Path"));

    auto result = cli.parse({argc, argv});

    if (!result) {
        std::cout << cli << endl;
        std::cerr << "Error in command line: " << result.message() << std::endl;

        return 1;
    }
    if (help || !result) {
        std::cout << cli << endl;

        return 0;
    }

    little_endian = !big_endian;

    cout << "File Low Path = " << fileL << endl;
    cout << "File High Path = " << fileH << endl;
    cout << "Output File Path = " << output << endl;
    cout << "Use Little Endian = " << (little_endian ? "true" : "false") << endl;
    cout << "Word Size = " << (int) word << endl;

    return combine_streams(fileL.c_str(), fileH.c_str(), output.c_str(), little_endian, word);
}
