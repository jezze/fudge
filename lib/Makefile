GCC=gcc
GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -m32
LD=ld
LDFLAGS=-melf_i386

.PHONY: all clean lib

all: lib

lib:
	@${GCC} ${GCCFLAGS} file.c -o file.o
	@${GCC} ${GCCFLAGS} memory.c -o memory.o
	@${GCC} ${GCCFLAGS} string.c -o string.o

clean:
	@rm -f *.o

