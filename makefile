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

BUILDDIR = build
SRCDIR = cthulhu

$(BUILDDIR)/cthulhu.o: $(SRCDIR)/cthulhu.c $(SRCDIR)/cthulhu.h
	@mkdir -p build
	$(CC) -c $(SRCDIR)/cthulhu.c -o $(BUILDDIR)/cthulhu.o $(CFLAGS)

cti: $(BUILDDIR)/cthulhu.o $(SRCDIR)/tools/interp.c
	$(CC) $(SRCDIR)/tools/interp.c -o $(BUILDDIR)/cti $(CFLAGS) $(BUILDDIR)/cthulhu.o

ctc: $(BUILDDIR)/cthulhu.o $(SRCDIR)/tools/compile.c
	$(CC) $(SRCDIR)/tools/compile.c -o $(BUILDDIR)/ctc $(CFLAGS) $(BUILDDIR)/cthulhu.o

test: $(BUILDDIR)/cthulhu.o $(SRCDIR)/tools/test.c
	$(CC) $(SRCDIR)/tools/test.c -o $(BUILDDIR)/test $(CFLAGS) $(BUILDDIR)/cthulhu.o
