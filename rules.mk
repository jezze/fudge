ARCH=x86
TARGET=
ASM=$(TARGET)as
GCC=$(TARGET)gcc
GCCFLAGS=-Wall -ffreestanding -nostdlib -std=c89 -pedantic
LD=$(TARGET)ld
AR=ar
ARFLAGS=rs
