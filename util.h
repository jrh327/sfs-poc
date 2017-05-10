#ifndef UTIL_H
#define UTIL_H

#include "includes.h"

/**
 * Read data from a file.
 * 
 * @param fd the file to read from
 * @param buf pointer to read into
 * @param length number of bytes to read
 */
void read_from_file(int fd, void* buf, uint64_t length);

/**
 * Write data to a file.
 * 
 * @param fd the file to write to
 * @param data pointer to the data to write
 * @param length number of bytes to write
 */
void write_to_file(int fd, const void* data, uint64_t length);

/**
 * Move to a position in a file.
 * 
 * @param fd the file to seek in
 * @param offset how far to move
 * @param mode whether to move from the beginning, end, or current position
 */
void seek_in_file(int fd, uint64_t offset, int mode);

/**
 * Close a file.
 * 
 * @param the file to close
 * @return success or failure
 */
int close_file(int fd);

/**
 * Get the length of a file.
 * 
 * @param fd the file whose length to get
 * @return the length of the file
 */
uint64_t tell_file(int fd);

/**
 * Put the bytes of a 16-bit value in big-endian order.
 * 
 * @param value the value to put in big-endian order
 * @return the result of the conversion
 */
uint16_t convert_uint16(uint16_t value);

/**
 * Put the bytes of a 32-bit value in big-endian order.
 * 
 * @param value the value to put in big-endian order
 * @return the result of the conversion
 */
uint32_t convert_uint32(uint32_t value);

/**
 * Put the bytes of a 64-bit value in big-endian order.
 * 
 * @param value the value to put in big-endian order
 * @return the result of the conversion
 */
uint64_t convert_uint64(uint64_t value);

/**
 * Read a 8-bit value from the given file.
 * 
 * @param fd the file to read from
 * @return the value that is read from the file
 */
uint8_t read_uint8(int fd);

/**
 * Read a 16-bit value from the given file.
 * 
 * The value is expected to be in big-endian format. This function will
 * convert to little-endian if needed by the current system.
 * 
 * @param fd the file to read from
 * @return the value that is read from the file
 */
uint16_t read_uint16(int fd);

/**
 * Read a 32-bit value from the given file.
 * 
 * The value is expected to be in big-endian format. This function will
 * convert to little-endian if needed by the current system.
 * 
 * @param fd the file to read from
 * @return the value that is read from the file
 */
uint32_t read_uint32(int fd);

/**
 * Read a 32-bit value from the given file.
 * 
 * The value is expected to be in big-endian format. This function will
 * convert to little-endian if needed by the current system.
 * 
 * @param fd the file to read from
 * @return the value that is read from the file
 */
uint64_t read_uint64(int fd);

/**
 * Write a 8-bit value to the given file.
 * 
 * @param fd the file to write to
 * @param value the value to write
 */
void write_uint8(int fd, uint8_t value);

/**
 * Write a 16-bit value to the given file.
 * 
 * The value will be written to the file in big-endian format. This function
 * will convert from little-endian if needed because of the current system.
 * 
 * @param fd the file to write to
 * @param value the value to write
 */
void write_uint16(int fd, uint16_t value);

/**
 * Write a 32-bit value to the given file.
 * 
 * The value will be written to the file in big-endian format. This function
 * will convert from little-endian if needed because of the current system.
 * 
 * @param fd the file to write to
 * @param value the value to write
 */
void write_uint32(int fd, uint32_t value);

/**
 * Write a 64-bit value to the given file.
 * 
 * The value will be written to the file in big-endian format. This function
 * will convert from little-endian if needed because of the current system.
 * 
 * @param fd the file to write to
 * @param value the value to write
 */
void write_uint64(int fd, uint64_t value);

/**
 * Retrieve a 16-bit value from the given byte array.
 * 
 * This function assumes the value in the array is in big-endian format, and
 * will convert to little-endian if needed because of the current system.
 * 
 * This function assumes pos + sizeof(uint16_t) does not extend past the end
 * of the array.
 * 
 * @param arr the array to read from
 * @param pos the position to start reading
 */
uint16_t get_uint16(uint8_t arr[], size_t pos);

/**
 * Retrieve a 32-bit value from the given byte array.
 * 
 * This function assumes the value in the array is in big-endian format, and
 * will convert to little-endian if needed because of the current system.
 * 
 * This function assumes pos + sizeof(uint32_t) does not extend past the end
 * of the array.
 * 
 * @param arr the array to read from
 * @param pos the position to start reading
 */
uint32_t get_uint32(uint8_t arr[], size_t pos);

/**
 * Retrieve a 64-bit value from the given byte array.
 * 
 * This function assumes the value in the array is in big-endian format, and
 * will convert to little-endian if needed because of the current system.
 * 
 * This function assumes pos + sizeof(uint64_t) does not extend past the end
 * of the array.
 * 
 * @param arr the array to read from
 * @param pos the position to start reading
 */
uint64_t get_uint64(uint8_t arr[], size_t pos);

/**
 * Insert a 16-bit value into the given byte array.
 * 
 * The value will be put into the array in big-endian format. This function
 * will convert from little-endian if needed because of the current system.
 * 
 * This function assumes pos + sizeof(value) does not extend past the end
 * of the array.
 * 
 * @param arr the array to insert into
 * @param value the value to insert
 * @param pos the position to start inserting
 */
void put_uint16(uint8_t arr[], uint16_t value, size_t pos);

/**
 * Insert a 32-bit value into the given byte array.
 * 
 * The value will be put into the array in big-endian format. This function
 * will convert from little-endian if needed because of the current system.
 * 
 * This function assumes pos + sizeof(value) does not extend past the end
 * of the array.
 * 
 * @param arr the array to insert into
 * @param value the value to insert
 * @param pos the position to start inserting
 */
void put_uint32(uint8_t arr[], uint32_t value, size_t pos);

/**
 * Insert a 64-bit value into the given byte array.
 * 
 * The value will be put into the array in big-endian format. This function
 * will convert from little-endian if needed because of the current system.
 * 
 * This function assumes pos + sizeof(value) does not extend past the end
 * of the array.
 * 
 * @param arr the array to insert into
 * @param value the value to insert
 * @param pos the position to start inserting
 */
void put_uint64(uint8_t arr[], uint64_t value, size_t pos);

#endif /* UTIL_H */
