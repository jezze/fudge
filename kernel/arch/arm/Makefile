ASM=arm-elf-as
ASMFLAGS=-c -mfpu=softfpa
GCC=arm-elf-gcc
GCCFLAGS=-c -O2 -I../../../include -Wall -Wextra -nostdlib -nostartfiles -ffreestanding -std=gnu99

.PHONY: all clean arch

all: arch

arch:
	@${GCC} ${GCCFLAGS} arch.c -o arch.o
	@${GCC} ${GCCFLAGS} call.c -o call.o
	@${ASM} ${ASMFLAGS} init.s -o init.o

clean:
	@rm -f *.o

