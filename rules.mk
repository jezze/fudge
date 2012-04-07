ROOT:=$(PWD)
PATH:=$(PATH):$(PWD)/toolchain/local/bin

ARCH=x86
TARGET=
ASM=${TARGET}as
ASMFLAGS=-c
GCC=${TARGET}gcc
GCCFLAGS=-c -O2 -Wall -ffreestanding -nostdlib -std=c99 -pedantic
LD=${TARGET}ld
AR=ar
ARFLAGS=rvs

