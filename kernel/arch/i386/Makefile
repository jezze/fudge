ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I../../../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs

all:
	@$(GCC) $(GCCFLAGS) arch.c -o arch.o
	@$(ASM) $(ASMFLAGS) cr.s -o cr.o
	@$(GCC) $(GCCFLAGS) gdt.c -o gdt.o
	@$(ASM) $(ASMFLAGS) gdts.s -o gdts.o
	@$(GCC) $(GCCFLAGS) idt.c -o idt.o
	@$(ASM) $(ASMFLAGS) idts.s -o idts.o
	@$(GCC) $(GCCFLAGS) io.c -o io.o
	@$(ASM) $(ASMFLAGS) irqs.s -o irqs.o
	@$(ASM) $(ASMFLAGS) isrs.s -o isrs.o
	@$(ASM) $(ASMFLAGS) loader.s -o loader.o

clean:
	@rm -f *.o

