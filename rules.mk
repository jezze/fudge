ROOT:=$(PWD)
PATH:=$(PATH):$(PWD)/toolchain/local/bin

ARCH=x86
TARGET=
ASM=${TARGET}as
ASMFLAGS=-c
GCC=${TARGET}gcc
GCCFLAGS=-c -Wall -ffreestanding -nostdlib -std=c89 -pedantic
LD=${TARGET}ld
AR=ar
ARFLAGS=rvs

