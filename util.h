#ifndef UTIL_H
#define UTIL_H 1

#include <stdint.h>

/**
 * Test for the endianness of the current system.
 * 
 * https://www.ibm.com/developerworks/aix/library/au-endianc/
 */
const int ENDIANNESS = 1;
#define IS_BIGENDIAN ((*(char*)&ENDIANNESS) == 0)

uint16_t convert_uint16(uint16_t value) {
    if (IS_BIGENDIAN) {
        return value;
    } else {
        uint8_t byte1 = (value & 0xff);
        uint8_t byte2 = (value >> 8) & 0xff;
        return (byte1 << 8) | byte2;
    }
}

uint32_t convert_uint32(uint32_t value) {
    if (IS_BIGENDIAN) {
        return value;
    } else {
        uint8_t byte1 = (value & 0xff);
        uint8_t byte2 = (value >> 8) & 0xff;
        uint8_t byte3 = (value >> 16) & 0xff;
        uint8_t byte4 = (value >> 24) & 0xff;
        return (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
    }
}

/**
 * Read a 16-bit value from the given file.
 * 
 * The value is expected to be in big-endian format. This function will
 * convert to little-endian if needed by the current system.
 * 
 * @param fp the file to read from
 * @return the value that is read from the file
 */
uint16_t read_uint16(FILE* fp) {
    uint16_t value;
    fread(&value, sizeof(value), 1, fp);
    return convert_uint16(value);
}

/**
 * Read a 32-bit value from the given file.
 * 
 * The value is expected to be in big-endian format. This function will
 * convert to little-endian if needed by the current system.
 * 
 * @param fp the file to read from
 * @return the value that is read from the file
 */
uint32_t read_uint32(FILE* fp) {
    uint32_t value;
    fread(&value, sizeof(value), 1, fp);
    return convert_uint32(value);
}

/**
 * Write a 16-bit value to the given file.
 * 
 * The value will be written to the file in big-endian format. This function
 * will convert from little-endian if needed because of the current system.
 * 
 * @param fp the file to write to
 * @param value the value to write
 */
void write_uint16(FILE* fp, uint16_t value) {
    value = convert_uint16(value);
    fwrite(&value, sizeof(value), 1, fp);
}

/**
 * Write a 32-bit value to the given file.
 * 
 * The value will be written to the file in big-endian format. This function
 * will convert from little-endian if needed because of the current system.
 * 
 * @param fp the file to write to
 * @param value the value to write
 */
void write_uint32(FILE* fp, uint32_t value) {
    value = convert_uint32(value);
    fwrite(&value, sizeof(value), 1, fp);
}

#endif /* UTIL_H */
