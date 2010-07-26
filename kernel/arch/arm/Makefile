ASM=arm-elf-as
ASMFLAGS=
GCC=arm-elf-gcc
GCCFLAGS=-c -I../../../include -Wall -Wextra -Werror -ffreestanding -nostdlib -nostartfiles -std=gnu99

all:
	@$(ASM) $(ASMFLAGS) loader.s -o loader.o
	@$(GCC) $(GCCFLAGS) arch.c -o arch.o

clean:
	@rm -f *.o


