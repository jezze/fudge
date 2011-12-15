all-arm: ARCH=arm
all-arm: GCC=arm-none-eabi-gcc
all-arm: GCCFLAGS=-c -O2 -I../include -Wall -Wextra -nostdlib -nostartfiles -ffreestanding -std=gnu99
all-x86: ARCH=x86
all-x86: GCC=gcc
all-x86: GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -m32

DIR_SOURCE_ARCH=arch/${ARCH}

.PHONY: all clean lib

all: all-x86
all-arm: lib arch-arm
all-x86: lib arch-x86

lib:
	@${GCC} ${GCCFLAGS} file.c -o file.o
	@${GCC} ${GCCFLAGS} memory.c -o memory.o
	@${GCC} ${GCCFLAGS} string.c -o string.o

arch-arm:
	@make -C ${DIR_SOURCE_ARCH}/

arch-x86:
	@make -C ${DIR_SOURCE_ARCH}/

clean:
	@make -C arch/arm/ clean
	@make -C arch/x86/ clean
	@rm -f *.o

