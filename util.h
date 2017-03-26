#ifndef UTIL_H
#define UTIL_H

#include "includes.h"

uint16_t convert_uint16(uint16_t value);

uint32_t convert_uint32(uint32_t value);

/**
 * Read a 16-bit value from the given file.
 * 
 * The value is expected to be in big-endian format. This function will
 * convert to little-endian if needed by the current system.
 * 
 * @param fp the file to read from
 * @return the value that is read from the file
 */
uint16_t read_uint16(FILE* fp);

/**
 * Read a 32-bit value from the given file.
 * 
 * The value is expected to be in big-endian format. This function will
 * convert to little-endian if needed by the current system.
 * 
 * @param fp the file to read from
 * @return the value that is read from the file
 */
uint32_t read_uint32(FILE* fp);

/**
 * Write a 16-bit value to the given file.
 * 
 * The value will be written to the file in big-endian format. This function
 * will convert from little-endian if needed because of the current system.
 * 
 * @param fp the file to write to
 * @param value the value to write
 */
void write_uint16(FILE* fp, uint16_t value);

/**
 * Write a 32-bit value to the given file.
 * 
 * The value will be written to the file in big-endian format. This function
 * will convert from little-endian if needed because of the current system.
 * 
 * @param fp the file to write to
 * @param value the value to write
 */
void write_uint32(FILE* fp, uint32_t value);

#endif /* UTIL_H */
