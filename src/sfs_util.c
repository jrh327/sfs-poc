/*
 * sfs_util.c
 *
 *  Created on: Aug 14, 2017
 *      Author: Jon Hopkins
 */

#include "sfs_includes.h"

/**
 * Test for the endianness of the current system.
 *
 * https://www.ibm.com/developerworks/aix/library/au-endianc/
 */
static const int ENDIANNESS = 1;
#define IS_BIGENDIAN ((*(char*)&ENDIANNESS) == 0)

HIDDEN SFS_FILE_LENGTH sfs_util_read_from_medium(int fd, void* buf,
        SFS_FILE_LENGTH length) {
    /* TODO: check for length > max ssize_t and in that case, loop with */
    /* chunks of length until all bytes read */
    uint8_t* ptr = buf;
    SFS_FILE_LENGTH bytes_left = length;
    while (bytes_left) {
        ssize_t bytes_read = read(fd, ptr, bytes_left);
        if (bytes_read == -1) {
            return (-1); /* error */
            /* TODO: consider also returning 0 here. SFS_FILE_LENGTH is */
            /* unsigned so -1 might not be appropriate to return, and 0 */
            /* is basically an error here since EOF should never happen */
        } else if (bytes_read == 0) {
            return (0); /* EOF */
        }
        bytes_left -= bytes_read;
        ptr += bytes_read;
    }
    return (length);
}

HIDDEN SFS_FILE_LENGTH sfs_util_write_to_medium(int fd, const void* data,
        SFS_FILE_LENGTH length) {
    /* TODO check for length > max ssize_t and in that case, loop with */
    /* chunks of length until all bytes written */
    const uint8_t* ptr = data;
    SFS_FILE_LENGTH bytes_left = length;
    while (bytes_left) {
        ssize_t bytes_written = write(fd, data, length);
        if (bytes_written == -1) {
            return (-1); /* error */
            /* TODO: consider also returning 0 here. SFS_FILE_LENGTH is */
            /* unsigned so -1 might not be appropriate to return, and 0 */
            /* should never happen here unless trying to write nothing. */
        }
        bytes_left -= bytes_written;
        ptr += bytes_written;
    }
    return (length);
}

HIDDEN SFS_FILE_OFFSET sfs_util_seek_in_medium(int fd, SFS_FILE_OFFSET offset,
        int mode) {
    return (lseek(fd, offset, mode));
}

HIDDEN int sfs_util_close_medium(int fd) {
    int err = close(fd);
    return (err);
}

HIDDEN SFS_FILE_OFFSET sfs_util_tell_file(int fd) {
    return (lseek(fd, 0, SEEK_CUR));
}

/* TODO just a possibility */
HIDDEN int sfs_util_eof(int fd) {
    int bytes_read = read(fd, 1, SEEK_CUR);
    if (bytes_read == -1) {
        return (-1);
    }
    if (bytes_read) {
        lseek(fd, -1, SEEK_CUR);
    }
    return (!bytes_read);
}

HIDDEN uint16_t sfs_util_convert_uint16(uint16_t value) {
    if (IS_BIGENDIAN) {
        return (value);
    } else {
        uint8_t byte1 = (value & 0xff);
        uint8_t byte2 = (value >> 8) & 0xff;
        return ((byte1 << 8) | byte2);
    }
}

HIDDEN uint32_t sfs_util_convert_uint32(uint32_t value) {
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

HIDDEN uint64_t sfs_util_convert_uint64(uint64_t value) {
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

HIDDEN uint8_t sfs_util_read_uint8(int fd) {
    uint8_t value;
    sfs_util_read_from_medium(fd, &value, sizeof(value));
    return (value);
}

HIDDEN uint16_t sfs_util_read_uint16(int fd) {
    uint16_t value;
    sfs_util_read_from_medium(fd, &value, sizeof(value));
    return (sfs_util_convert_uint16(value));
}

HIDDEN uint32_t sfs_util_read_uint32(int fd) {
    uint32_t value;
    sfs_util_read_from_medium(fd, &value, sizeof(value));
    return (sfs_util_convert_uint32(value));
}

HIDDEN uint64_t sfs_util_read_uint64(int fd) {
    uint64_t value;
    sfs_util_read_from_medium(fd, &value, sizeof(value));
    return (sfs_util_convert_uint64(value));
}

HIDDEN void sfs_util_write_uint8(int fd, uint8_t value) {
    sfs_util_write_to_medium(fd, &value, sizeof(value));
}

HIDDEN void sfs_util_write_uint16(int fd, uint16_t value) {
    value = sfs_util_convert_uint16(value);
    sfs_util_write_to_medium(fd, &value, sizeof(value));
}

HIDDEN void sfs_util_write_uint32(int fd, uint32_t value) {
    value = sfs_util_convert_uint32(value);
    sfs_util_write_to_medium(fd, &value, sizeof(value));
}

HIDDEN void sfs_util_write_uint64(int fd, uint64_t value) {
    value = sfs_util_convert_uint64(value);
    sfs_util_write_to_medium(fd, &value, sizeof(value));
}

HIDDEN uint16_t sfs_util_get_uint16(uint8_t arr[], size_t pos) {
    uint16_t value = 0;
    for (size_t i = 0; i < 2; i++) {
        value = (value << 8) | arr[pos + i];
    }
    return (value);
}

HIDDEN uint32_t sfs_util_get_uint32(uint8_t arr[], size_t pos) {
    uint32_t value = 0;
    for (size_t i = 0; i < 4; i++) {
        value = (value << 8) | arr[pos + i];
    }
    return (value);
}

HIDDEN uint64_t sfs_util_get_uint64(uint8_t arr[], size_t pos) {
    uint64_t value = 0;
    for (size_t i = 0; i < 8; i++) {
        value = (value << 8) | arr[pos + i];
    }
    return (value);
}

HIDDEN void sfs_util_put_uint16(uint8_t arr[], uint16_t value, size_t pos) {
    arr[pos] = (value >> 8) & 0xff;
    arr[pos + 1] = (value & 0xff);
}

HIDDEN void sfs_util_put_uint32(uint8_t arr[], uint32_t value, size_t pos) {
    arr[pos] = (value >> 24) & 0xff;
    arr[pos + 1] = (value >> 16) & 0xff;
    arr[pos + 2] = (value >> 8) & 0xff;
    arr[pos + 3] = (value & 0xff);
}

HIDDEN void sfs_util_put_uint64(uint8_t arr[], uint64_t value, size_t pos) {
    arr[pos] = (value >> 56) & 0xff;
    arr[pos + 1] = (value >> 48) & 0xff;
    arr[pos + 2] = (value >> 40) & 0xff;
    arr[pos + 3] = (value >> 32) & 0xff;
    arr[pos + 4] = (value >> 24) & 0xff;
    arr[pos + 5] = (value >> 16) & 0xff;
    arr[pos + 6] = (value >> 8) & 0xff;
    arr[pos + 7] = (value & 0xff);
}

HIDDEN int sfs_util_empty(uint8_t arr[], size_t length) {
    for (size_t i = 0; i < length; i++) {
        if (arr[i] != 0) {
            return (0);
        }
    }
    return (1);
}
