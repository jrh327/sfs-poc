/*
 * test.c
 *
 *  Created on: Sep 1, 2017
 *      Author: Jon Hopkins
 */

#include "test.h"
#include "test_util_functions.h"
#include "test_sfs_functions.h"
#include "test_dir_functions.h"
#include "test_file_functions.h"

const char* filename = "test3.bin";

int create_tmp_file() {
    int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        printf("error creating file\n");
        abort();
    }
    return (fd);
}

int reopen_tmp_file() {
    int fd = open(filename, O_RDWR);
    if (fd < 0) {
        printf("error reading file\n");
        return (-1);
    }
    return (fd);
}

void delete_tmp_file() {
    int err = remove(filename);
    if (err != 0) {
        printf("Could not delete file\n");
        abort();
    }
}

void run_test(char* test_name, int (*test)()) {
    printf("--------------------\n");
    int result = test();
    if (result != 0) {
        printf("%s() failed\n", test_name);
    } else {
        printf("%s() passed\n", test_name);
    }
    printf("--------------------\n\n");
}

int run_tests() {
    run_test("test_writing_uint16", test_writing_uint16);
    run_test("test_reading_uint16", test_reading_uint16);
    run_test("test_new_bootsector_constraints",
     test_new_bootsector_constraints);
    run_test("test_reading_directory_entry", test_reading_directory_entry);
    run_test("test_read_dir_entry_short_filename",
            test_read_dir_entry_short_filename);
    run_test("test_read_dir_entry_long_filename",
            test_read_dir_entry_long_filename);
    run_test("test_create_file", test_create_file);
    run_test("test_read_file", test_read_file);

    /* test_aes(0, NULL); */
    return (0);
}

int main(int argc, char** argv) {
    run_tests();
    return (0);
}
