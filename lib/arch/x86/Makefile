ASM=as
ASMFLAGS=-c
GCC=gcc
GCCFLAGS=-c -O2 -I../../../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -m32

.PHONY: all clean arch

all: arch

arch:
	@${ASM} ${ASMFLAGS} call.s -o call.o
	@${ASM} ${ASMFLAGS} crt0.s -o crt0.o

clean:
	@rm -f *.o

