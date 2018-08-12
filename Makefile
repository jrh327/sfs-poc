MAIN_TARGET = main
TEST_TARGET = test

MAIN_OUTPUT = sfs
TEST_OUTPUT = sfs_test

CC = gcc
CFLAGS = -Wall
LIBS = -lm

HEADERS = src/sfs_includes.h src/sfs_structs.h src/sfs_util.h src/sfs.h src/sfs_directory.h src/sfs_encryption.h src/sfs_fat.h src/sfs_file.h src/sfs_io.h
SOURCES = src/sfs_structs.c src/sfs_util.c src/sfs.c src/sfs_directory.c src/sfs_encryption.c src/sfs_fat.c src/sfs_file.c src/sfs_io.c
MAIN_SOURCE = src/main.c
TEST_HEADERS = test/test.h test/test_dir_functions.h test/test_file_functions.h test/test_sfs_functions.h test/test_util_functions.h
TEST_SOURCE = test/test.c test/test_dir_functions.c test/test_file_functions.c test/test_sfs_functions.c test/test_util_functions.c

default: $(MAIN_TARGET)

$(MAIN_TARGET): $(HEADERS) $(SOURCES) $(MAIN_SOURCE)
	$(CC) $(CFLAGS) $(SOURCES) $(MAIN_SOURCE) -o $(MAIN_OUTPUT)

$(TEST_TARGET): $(HEADERS) $(TEST_HEADERS) $(SOURCES) $(TEST_SOURCE)
	$(CC) $(CFLAGS) $(SOURCES) $(TEST_SOURCE) -o $(TEST_OUTPUT) && ./$(TEST_OUTPUT)

clean:
	-rm -f $(MAIN_OUTPUT)
	-rm -f $(TEST_OUTPUT)
