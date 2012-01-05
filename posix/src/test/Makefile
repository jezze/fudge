all: GCC=gcc
all: GCCFLAGS=-O2 -I../../include -I../../../include/lib -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -L../../lib -L../../../lib -m32

.PHONY: all clean

all:
	@${GCC} ${GCCFLAGS} main.c -lfudge -o test

clean:
	@rm -f *.o
	@rm -f test

