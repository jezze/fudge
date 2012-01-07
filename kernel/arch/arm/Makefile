include ../../../rules.mk

ASMFLAGS=-c -mfpu=softfpa
GCCFLAGS=-c -O2 -I../../../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs

.PHONY: all clean arch

all: arch

arch:
	@${GCC} ${GCCFLAGS} arch.c -o arch.o
	@${ASM} ${ASMFLAGS} cpu.s -o cpu.o
	@${ASM} ${ASMFLAGS} init.s -o init.o

clean:
	@rm -f *.o

