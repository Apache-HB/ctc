CC = clang

CFLAGS = -Wextra -Weverything -Wall -Werror -Wno-padded -pedantic

all:
	mkdir -p build
	$(CC) ctc/ctc.c -c $(CFLAGS) -o build/ctc.o