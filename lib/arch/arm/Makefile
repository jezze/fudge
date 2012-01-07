include ../../../rules.mk

ASMFLAGS=-c -mfpu=softfpa
GCCFLAGS=-c -O2 -I../../../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs

.PHONY: all clean arch

all: arch

arch:
	@${GCC} ${GCCFLAGS} call.c -o call.o
	@${ASM} ${ASMFLAGS} crt0.s -o crt0.o

clean:
	@rm -f *.o

