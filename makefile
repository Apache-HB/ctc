CC = clang

CFLAGS = -Wextra -Weverything -Wall -Werror -Wno-padded -pedantic

all:
	$(CC) ctc.c -c $(CFLAGS)