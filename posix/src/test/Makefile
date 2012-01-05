all: GCC=gcc
all: GCCFLAGS=-O2 -I../../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -L../../lib -m32

.PHONY: all clean

all:
	@${GCC} ${GCCFLAGS} -lc -lfudge main.c -o test

clean:
	@rm -f *.o
	@rm -f test

