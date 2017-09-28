/*
 * sfs_util.h
 *
 * Defines utility functions for low-level things that the rest of the code
 * shouldn't have to worry about.
 *
 *  Created on: Aug 14, 2017
 *      Author: Jon Hopkins
 */

#ifndef SFS_UTIL_H
#define SFS_UTIL_H

#include "sfs_includes.h"

/**
 * Read data from the underlying medium.
 *
 * @param fd file descriptor to read from
 * @param buf pointer to read into
 * @param length number of bytes to read
 * @return the number of bytes read
 */
HIDDEN SFS_FILE_LENGTH sfs_util_read_from_medium(int fd, void* buf,
        SFS_FILE_LENGTH length);

/**
 * Write data to the underlying medium.
 *
 * @param fd file descriptor to write to
 * @param data pointer to the data to write
 * @param length number of bytes to write
 * @return the number of bytes written
 */
HIDDEN SFS_FILE_LENGTH sfs_util_write_to_medium(int fd, const void* data,
        SFS_FILE_LENGTH length);

/**
 * Move to a position within the underlying medium.
 *
 * @param fd file descriptor to seek within
 * @param offset how far to move
 * @param mode whether to move from the beginning, end, or current position
 * @return the position in the file
 */
HIDDEN SFS_FILE_OFFSET sfs_util_seek_in_medium(int fd, SFS_FILE_OFFSET offset,
        int mode);

/**
 * Close the connection to the underlying medium.
 *
 * @param fd file descriptor to close
 * @return success or failure
 */
HIDDEN int sfs_util_close_medium(int fd);

/**
 * Get the length of a file.
 *
 * @param fd the file whose length to get
 * @return the length of the file
 */
HIDDEN SFS_FILE_OFFSET sfs_util_tell_file(int fd);

/**
 * Put the bytes of a 16-bit value in big-endian order.
 *
 * @param value the value to put in big-endian order
 * @return the result of the conversion
 */
HIDDEN uint16_t sfs_util_convert_uint16(uint16_t value);

/**
 * Put the bytes of a 32-bit value in big-endian order.
 *
 * @param value the value to put in big-endian order
 * @return the result of the conversion
 */
HIDDEN uint32_t sfs_util_convert_uint32(uint32_t value);

/**
 * Put the bytes of a 64-bit value in big-endian order.
 *
 * @param value the value to put in big-endian order
 * @return the result of the conversion
 */
HIDDEN uint64_t sfs_util_convert_uint64(uint64_t value);

/**
 * Read a 8-bit value from the given file.
 *
 * @param fd the file to read from
 * @return the value that is read from the file
 */
HIDDEN uint8_t sfs_util_read_uint8(int fd);

/**
 * Read a 16-bit value from the given file.
 *
 * The value is expected to be in big-endian format. This function will
 * convert to little-endian if needed by the current system.
 *
 * @param fd the file to read from
 * @return the value that is read from the file
 */
HIDDEN uint16_t sfs_util_read_uint16(int fd);

/**
 * Read a 32-bit value from the given file.
 *
 * The value is expected to be in big-endian format. This function will
 * convert to little-endian if needed by the current system.
 *
 * @param fd the file to read from
 * @return the value that is read from the file
 */
HIDDEN uint32_t sfs_util_read_uint32(int fd);

/**
 * Read a 32-bit value from the given file.
 *
 * The value is expected to be in big-endian format. This function will
 * convert to little-endian if needed by the current system.
 *
 * @param fd the file to read from
 * @return the value that is read from the file
 */
HIDDEN uint64_t sfs_util_read_uint64(int fd);

/**
 * Write a 8-bit value to the given file.
 *
 * @param fd the file to write to
 * @param value the value to write
 */
HIDDEN void sfs_util_write_uint8(int fd, uint8_t value);

/**
 * Write a 16-bit value to the given file.
 *
 * The value will be written to the file in big-endian format. This function
 * will convert from little-endian if needed because of the current system.
 *
 * @param fd the file to write to
 * @param value the value to write
 */
HIDDEN void sfs_util_write_uint16(int fd, uint16_t value);

/**
 * Write a 32-bit value to the given file.
 *
 * The value will be written to the file in big-endian format. This function
 * will convert from little-endian if needed because of the current system.
 *
 * @param fd the file to write to
 * @param value the value to write
 */
HIDDEN void sfs_util_write_uint32(int fd, uint32_t value);

/**
 * Write a 64-bit value to the given file.
 *
 * The value will be written to the file in big-endian format. This function
 * will convert from little-endian if needed because of the current system.
 *
 * @param fd the file to write to
 * @param value the value to write
 */
HIDDEN void sfs_util_write_uint64(int fd, uint64_t value);

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
HIDDEN uint16_t sfs_util_get_uint16(uint8_t arr[], size_t pos);

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
HIDDEN uint32_t sfs_util_get_uint32(uint8_t arr[], size_t pos);

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
HIDDEN uint64_t sfs_util_get_uint64(uint8_t arr[], size_t pos);

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
HIDDEN void sfs_util_put_uint16(uint8_t arr[], uint16_t value, size_t pos);

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
HIDDEN void sfs_util_put_uint32(uint8_t arr[], uint32_t value, size_t pos);

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
HIDDEN void sfs_util_put_uint64(uint8_t arr[], uint64_t value, size_t pos);

/**
 * Check if the byte array contains only 0s.
 *
 * @param arr the array to check
 * @param length the number of bytes to check
 * @return 1 if only 0s, otherwise 0
 */
HIDDEN int sfs_util_empty(uint8_t arr[], size_t length);

#endif /* SFS_UTIL_H */
