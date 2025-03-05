#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <ostream>

using namespace std;

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

int combine_streams(const char *fileL, const char *fileH, const char *output_file, bool little_endian) {
    uint8_t *data[2];
    uint32_t fsize[2];
    cout << "Reading file " << fileL << endl;
    fsize[0] = read_file(fileL, &data[0]);
    cout << "File Low Size = " << fsize[0] << endl;
    cout << "Reading file " << fileH << endl;
    fsize[1] = read_file(fileH, &data[1]);
    cout << "File High Size = " << fsize[1] << endl;

    if (fsize[0] != fsize[1])
        cout << "Error: File sizes are not the same. Exiting!" << endl;
        return -1; // file sizes are mismatching

    uint32_t size = fsize[0];
    uint16_t *data_out = (uint16_t *) malloc(sizeof(uint16_t) * size);
    memset(data_out, 0, sizeof(uint16_t) * size);

    uint8_t *dataL = (little_endian ? data[0] : data[1]);
    uint8_t *dataH = (little_endian ? data[1] : data[0]);
    for (uint32_t i = 0; i < size; ++i) {
        data_out[i] = (dataH[i] << 8) | dataL[i];
    }

    FILE *out_file = fopen(output_file, "wb");
    fwrite(data_out, sizeof(uint16_t), size, out_file);
    fclose(out_file);

    free(data_out);
    free(data[0]);
    free(data[1]);
    return 0;
}

int main(int argc, char *argv[]) {
    return combine_streams("/data/Projects/BinaryInterleave/fileL.bin", "/data/Projects/BinaryInterleave/fileH.bin", "/data/Projects/BinaryInterleave/combined.bin", true);
}
