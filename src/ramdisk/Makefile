ASM=nasm
ASMFLAGS=-f bin
GCC=gcc
GCCFLAGS=-O2 -I./include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
OBJCOPY=objcopy
OBJCOPYFLAGS=-O binary

all:
	$(GCC) $(GCCFLAGS) hello2.c -o hello2elf
	$(OBJCOPY) $(OBJCOPYFLAGS) hello2elf hello2
	rm -f hello2elf
	$(ASM) $(ASMFLAGS) hello.s -o hello

clean:
	rm -f hello
	rm -f hello2
