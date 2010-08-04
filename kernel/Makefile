ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs

all:
	@$(GCC) $(GCCFLAGS) assert.c -o assert.o
	@$(GCC) $(GCCFLAGS) elf.c -o elf.o
	@$(GCC) $(GCCFLAGS) initrd.c -o initrd.o
	@$(GCC) $(GCCFLAGS) kernel.c -o kernel.o
	@$(GCC) $(GCCFLAGS) mboot.c -o mboot.o
	@$(GCC) $(GCCFLAGS) shell.c -o shell.o

clean:
	@rm -f *.o

