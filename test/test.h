/*
 * test.h
 *
 *  Created on: Sep 3, 2017
 *      Author: Jon Hopkins
 */

#ifndef TEST_H
#define TEST_H

#include "../src/sfs_includes.h"

int create_tmp_file();

int reopen_tmp_file();

void delete_tmp_file();

int run_tests();

void run_test(char* test_name, int (*test)());

#endif /* TEST_H */
