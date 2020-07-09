CC = clang-10

CFLAGS = -Wextra -Weverything -Wall -Werror \
	-Wno-padded -Wno-switch-enum -Wno-format-nonliteral \
	-pedantic -std=c99 -g \
	-DPRINTF_DISABLE_SUPPORT_LONG_LONG \
	-DPRINTF_DISABLE_SUPPORT_FLOAT \
	-I. \
	-mno-x87 \
	-fno-threadsafe-statics \
	-ffreestanding \
	-Wno-missing-prototypes

setup:
	mkdir -p build

test: setup
	$(CC) ctc/tests/test.c -o build/test $(CFLAGS)

interp: setup
	$(CC) ctc/tests/interp.c -o build/interp $(CFLAGS)