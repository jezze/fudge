include ../../../rules.mk
include ../../../${ARCH}.mk

GCCFLAGS+=-I../../../include

.PHONY: all clean arch

all: arch

arch:
	@${GCC} ${GCCFLAGS} arch.c -o arch.o
	@${ASM} ${ASMFLAGS} cpu.s -o cpu.o
	@${ASM} ${ASMFLAGS} init.s -o init.o

clean:
	@rm -f *.o

