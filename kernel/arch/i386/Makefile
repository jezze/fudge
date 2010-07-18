ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I../../../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs

all:
	@$(ASM) $(ASMFLAGS) cr.s -o cr.o
	@$(ASM) $(ASMFLAGS) gdt.s -o gdt.o
	@$(ASM) $(ASMFLAGS) idt.s -o idt.o
	@$(GCC) $(GCCFLAGS) io.c -o io.o
	@$(ASM) $(ASMFLAGS) irq.s -o irq.o
	@$(ASM) $(ASMFLAGS) isr.s -o isr.o
	@$(ASM) $(ASMFLAGS) loader.s -o loader.o

clean:
	@rm -f *.o

