MAIN_TARGET = main
TEST_TARGET = test

CC = gcc
CFLAGS = -Wall
LIBS = -lm

HEADERS = includes.h files.h sfs.h util.h
SOURCES = files.c sfs.c util.c
MAIN_SOURCE = main.c
TEST_SOURCE = test.c

default: $(MAIN_TARGET)

$(MAIN_TARGET): $(HEADERS) $(SOURCES) $(MAIN_SOURCE)
	$(CC) $(CFLAGS) $(SOURCES) $(MAIN_SOURCE) -o $@

$(TEST_TARGET): $(HEADERS) $(SOURCES) $(TEST_SOURCE)
	$(CC) $(CFLAGS) $(SOURCES) $(TEST_SOURCE) -o $@

clean:
	-rm -f $(MAIN_TARGET)
	-rm -f $(TEST_TARGET)
