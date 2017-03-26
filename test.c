#include <stdlib.h>
#include <stdio.h>
#include "sfs.h"

const char* filename = "test.bin";

void delete_file() {
    int err = remove(filename);
    if (err != 0) {
        printf("Could not delete file\n");
    }
}

int test_writing_uint16() {
    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("error creating file\n");
        return -1;
    }
    const uint16_t test_val = 0x1234;
    write_uint16(fp, test_val);
    fclose(fp);

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("error reading file\n");
        return -1;
    }

    int ret = 0;
    uint8_t byte;
    fread(&byte, sizeof byte, 1, fp);
    if (byte != 0x12) {
        printf("first byte - expected: %d, got %d\n", 0x12, byte);
        ret = -1;
    }

    fread(&byte, sizeof byte, 1, fp);
    if (byte != 0x34) {
        printf("second byte - expected: %d, got %d\n", 0x34, byte);
        ret = -1;
    }
    fclose(fp);

    delete_file();

    return ret;
}

int test_reading_uint16() {
    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("error creating file\n");
        return -1;
    }

    uint8_t byte = 0x12;
    fwrite(&byte, sizeof byte, 1, fp);
    byte = 0x34;
    fwrite(&byte, sizeof byte, 1, fp);
    fclose(fp);

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("error reading file\n");
        return -1;
    }

    int ret = 0;
    const uint16_t test_val = 0x1234;
    uint16_t read_val = read_uint16(fp);
    if (read_val != test_val) {
        printf("read value - expected: %d, got %d\n", test_val, read_val);
        ret = -1;
    }
    fclose(fp);

    delete_file();

    return ret;
}

int main() {
    if (test_writing_uint16() != 0) {
        printf("test_writing_uint16() failed\n");
    } else {
        printf("test_writing_uint16() passed\n");
    }

    if (test_reading_uint16() != 0) {
        printf("test_reading_uint16() failed\n");
    } else {
        printf("test_reading_uint16() passed\n");
    }
}
