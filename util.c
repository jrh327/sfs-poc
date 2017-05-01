#include "util.h"

/**
 * Test for the endianness of the current system.
 * 
 * https://www.ibm.com/developerworks/aix/library/au-endianc/
 */
const int ENDIANNESS = 1;
#define IS_BIGENDIAN ((*(char*)&ENDIANNESS) == 0)

uint16_t convert_uint16(uint16_t value) {
    if (IS_BIGENDIAN) {
        return (value);
    } else {
        uint8_t byte1 = (value & 0xff);
        uint8_t byte2 = (value >> 8) & 0xff;
        return ((byte1 << 8) | byte2);
    }
}

uint32_t convert_uint32(uint32_t value) {
    if (IS_BIGENDIAN) {
        return (value);
    } else {
        uint8_t byte1 = (value & 0xff);
        uint8_t byte2 = (value >> 8) & 0xff;
        uint8_t byte3 = (value >> 16) & 0xff;
        uint8_t byte4 = (value >> 24) & 0xff;
        return ((byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4);
    }
}

uint64_t convert_uint64(uint64_t value) {
    if (IS_BIGENDIAN) {
        return (value);
    } else {
        uint8_t byte1 = (value & 0xff);
        uint8_t byte2 = (value >> 8) & 0xff;
        uint8_t byte3 = (value >> 16) & 0xff;
        uint8_t byte4 = (value >> 24) & 0xff;
        uint8_t byte5 = (value >> 32) & 0xff;
        uint8_t byte6 = (value >> 40) & 0xff;
        uint8_t byte7 = (value >> 48) & 0xff;
        uint8_t byte8 = (value >> 56) & 0xff;

        value = 0;
        value = value | byte1;
        value = (value << 8) | byte2;
        value = (value << 8) | byte3;
        value = (value << 8) | byte4;
        value = (value << 8) | byte5;
        value = (value << 8) | byte6;
        value = (value << 8) | byte7;
        value = (value << 8) | byte8;
        return (value);
    }
}

uint8_t read_uint8(FILE* fp) {
    uint8_t value;
    fread(&value, sizeof(value), 1, fp);
    return (value);
}

uint16_t read_uint16(FILE* fp) {
    uint16_t value;
    fread(&value, sizeof(value), 1, fp);
    return (convert_uint16(value));
}

uint32_t read_uint32(FILE* fp) {
    uint32_t value;
    fread(&value, sizeof(value), 1, fp);
    return (convert_uint32(value));
}

uint64_t read_uint64(FILE* fp) {
    uint64_t value;
    fread(&value, sizeof(value), 1, fp);
    return (convert_uint64(value));
}

void write_uint8(FILE* fp, uint8_t value) {
    fwrite(&value, sizeof(value), 1, fp);
}

void write_uint16(FILE* fp, uint16_t value) {
    value = convert_uint16(value);
    fwrite(&value, sizeof(value), 1, fp);
}

void write_uint32(FILE* fp, uint32_t value) {
    value = convert_uint32(value);
    fwrite(&value, sizeof(value), 1, fp);
}

void write_uint64(FILE* fp, uint64_t value) {
    value = convert_uint64(value);
    fwrite(&value, sizeof(value), 1, fp);
}

uint16_t get_uint16(uint8_t arr[], size_t pos) {
    uint16_t value = 0;
    for (size_t i = 0; i < 2; i++) {
        value = (value << 8) | arr[pos + i];
    }
    return (value);
}

uint32_t get_uint32(uint8_t arr[], size_t pos) {
    uint32_t value = 0;
    for (size_t i = 0; i < 4; i++) {
        value = (value << 8) | arr[pos + i];
    }
    return (value);
}

uint64_t get_uint64(uint8_t arr[], size_t pos) {
    uint64_t value = 0;
    for (size_t i = 0; i < 8; i++) {
        value = (value << 8) | arr[pos + i];
    }
    return (value);
}

void put_uint16(uint8_t arr[], uint16_t value, size_t pos) {
    arr[pos] = (value >> 8) & 0xff;
    arr[pos + 1] = (value & 0xff);
}

void put_uint32(uint8_t arr[], uint32_t value, size_t pos) {
    arr[pos] = (value >> 24) & 0xff;
    arr[pos + 1] = (value >> 16) & 0xff;
    arr[pos + 2] = (value >> 8) & 0xff;
    arr[pos + 3] = (value & 0xff);
}

void put_uint64(uint8_t arr[], uint64_t value, size_t pos) {
    //value = convert_uint64(value);
    arr[pos] = (value >> 56) & 0xff;
    arr[pos + 1] = (value >> 48) & 0xff;
    arr[pos + 2] = (value >> 40) & 0xff;
    arr[pos + 3] = (value >> 32) & 0xff;
    arr[pos + 4] = (value >> 24) & 0xff;
    arr[pos + 5] = (value >> 16) & 0xff;
    arr[pos + 6] = (value >> 8) & 0xff;
    arr[pos + 7] = (value & 0xff);
}
