ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
LD=ld
LDFLAGS=-T ../linker.ld

all:
	@$(GCC) $(GCCFLAGS) call.c -o call.o
	@$(ASM) $(ASMFLAGS) calls.s -o calls.o
	@$(GCC) $(GCCFLAGS) cbuffer.c -o cbuffer.o
	@$(GCC) $(GCCFLAGS) io.c -o io.o
	@$(GCC) $(GCCFLAGS) memory.c -o memory.o
	@$(GCC) $(GCCFLAGS) stack.c -o stack.o
	@$(GCC) $(GCCFLAGS) string.c -o string.o
	@$(GCC) $(GCCFLAGS) vfs.c -o vfs.o

clean:
	@echo "Cleaning..."
	@rm -f *.o

