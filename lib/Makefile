all-arm: ARCH=arm
all-arm: GCC=arm-elf-gcc
all-arm: GCCFLAGS=-c -O2 -I../include -Wall -Wextra -nostdlib -nostartfiles -ffreestanding -std=gnu99
all-x86: ARCH=x86
all-x86: GCC=gcc
all-x86: GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -m32

.PHONY: all clean lib

all: all-x86
all-arm: lib
all-x86: lib

lib:
	@${GCC} ${GCCFLAGS} file.c -o file.o
	@${GCC} ${GCCFLAGS} memory.c -o memory.o
	@${GCC} ${GCCFLAGS} string.c -o string.o

clean:
	@rm -f *.o

