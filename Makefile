MAIN_TARGET = main
TEST_TARGET = test
TEST_TWOFISH = twofish

CC = gcc
CFLAGS = -Wall
LIBS = -lm

HEADERS = includes.h allocation.h files.h sfs.h structs.h util.h encryption/aes.h encryption/debug.h encryption/platform.h encryption/table.h
SOURCES = allocation.c files.c sfs.c structs.c util.c encryption/twofish2.c encryption/tst2fish.c
MAIN_SOURCE = main.c
TEST_SOURCE = test.c
TWOFISH_HEADERS = encryption/aes.h encryption/debug.h encryption/platform.h encryption/table.h
TWOFISH_SOURCES = encryption/twofish2.c encryption/tst2fish.c

default: $(MAIN_TARGET)

$(MAIN_TARGET): $(HEADERS) $(SOURCES) $(MAIN_SOURCE)
	$(CC) $(CFLAGS) $(SOURCES) $(MAIN_SOURCE) -o $@

$(TEST_TARGET): $(HEADERS) $(SOURCES) $(TEST_SOURCE)
	$(CC) $(CFLAGS) $(SOURCES) $(TEST_SOURCE) -o $@ && ./$(TEST_TARGET)

$(TEST_TWOFISH): $(TWOFISH_HEADERS) $(TWOFISH_SOURCES)
	$(CC) $(CFLAGS) $(TWOFISH_SOURCES) -o $@ && ./$(TEST_TWOFISH)

clean:
	-rm -f $(MAIN_TARGET)
	-rm -f $(TEST_TARGET)
	-rm -f $(TEST_TWOFISH)
	-rm -f cbc_*.txt
	-rm -f ecb_*.txt
