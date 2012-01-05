all: GCC=gcc
all: GCCFLAGS=-static -s -O2 -I../../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -L../../lib -L/usr/lib/gcc/i686-pc-linux-gnu/4.6.2/ -m32

.PHONY: all clean

all:
	@${GCC} ${GCCFLAGS} main.c -lc -lfudge -lgcc -o test

clean:
	@rm -f *.o
	@rm -f test

