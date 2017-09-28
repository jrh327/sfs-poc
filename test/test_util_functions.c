/*
 * test_util_functions.c
 *
 *  Created on: Sep 3, 2017
 *      Author: Jon Hopkins
 */

#include "test.h"

int test_writing_uint16() {
    int fd = create_tmp_file();

    const uint16_t test_val = 0x1234;
    sfs_util_write_uint16(fd, test_val);
    sfs_util_close_medium(fd);

    fd = reopen_tmp_file();

    int ret = 0;
    uint8_t byte;
    sfs_util_read_from_medium(fd, &byte, sizeof(byte));
    if (byte != 0x12) {
        printf("first byte - expected: %d, got %d\n", 0x12, byte);
        ret = -1;
    }

    sfs_util_read_from_medium(fd, &byte, sizeof(byte));
    if (byte != 0x34) {
        printf("second byte - expected: %d, got %d\n", 0x34, byte);
        ret = -1;
    }
    sfs_util_close_medium(fd);

    delete_tmp_file();

    return (ret);
}

int test_reading_uint16() {
    int fd = create_tmp_file();

    uint8_t byte = 0x12;
    sfs_util_write_to_medium(fd, &byte, sizeof(byte));
    byte = 0x34;
    sfs_util_write_to_medium(fd, &byte, sizeof(byte));
    sfs_util_close_medium(fd);

    fd = reopen_tmp_file();

    int ret = 0;
    const uint16_t test_val = 0x1234;
    uint16_t read_val = sfs_util_read_uint16(fd);
    if (read_val != test_val) {
        printf("read value - expected: %d, got %d\n", test_val, read_val);
        ret = -1;
    }
    sfs_util_close_medium(fd);

    delete_tmp_file();

    return (ret);
}
