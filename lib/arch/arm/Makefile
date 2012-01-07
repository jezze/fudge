include ../../../rules.mk
include ../../../${ARCH}.mk

GCCFLAGS+=-I../../../include

.PHONY: all clean arch

all: arch

arch:
	@${GCC} ${GCCFLAGS} call.c -o call.o
	@${ASM} ${ASMFLAGS} crt0.s -o crt0.o

clean:
	@rm -f *.o

