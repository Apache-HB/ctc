CC = clang

CFLAGS = -Wextra -Weverything -Wall -Werror -Wno-padded -Wno-format-nonliteral -pedantic -std=c89 -g

all:
	mkdir -p build
	$(CC) ctc/ctc.c -c $(CFLAGS) -o build/ctc.o

test: all
	$(CC) ctc/test.c build/ctc.o -o build/test $(CFLAGS)