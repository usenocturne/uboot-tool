CC=gcc
CFLAGS=-Wall -static -static-libgcc

.PHONY: all
all: clean uboot_tool

clean:
	rm -f uboot_tool

uboot_tool:
	$(CC) $(CFLAGS) -o uboot_tool src/uboot.c
