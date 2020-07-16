CC = clang-10

CFLAGS = -Wextra -Weverything -Wall -Werror \
	-Wno-padded -Wno-switch-enum -Wno-format-nonliteral \
	-pedantic -std=c99 -g \
	-DPRINTF_DISABLE_SUPPORT_LONG_LONG \
	-DPRINTF_DISABLE_SUPPORT_FLOAT \
	-I. \
	-mno-x87 \
	-fno-threadsafe-statics \
	-ffreestanding

setup:
	mkdir -p build

interp: setup
	$(CC) ctc/tools/interp.c -o build/cti $(CFLAGS)

compile: setup
	$(CC) ctc/tools/compile.c -o build/ctc $(CFLAGS)
