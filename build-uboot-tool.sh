#!/bin/sh

arm-linux-gnueabi-gcc -Wall -static -static-libgcc -o uboot_tool resources/uboot.c

mkdir -p deploy
install uboot_tool deploy/uboot_tool
