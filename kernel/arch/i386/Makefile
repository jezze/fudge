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
	@$(GCC) $(GCCFLAGS) initrd.c -o initrd.o
	@$(GCC) $(GCCFLAGS) io.c -o io.o
	@$(GCC) $(GCCFLAGS) irq.c -o irq.o
	@$(ASM) $(ASMFLAGS) irqs.s -o irqs.o
	@$(GCC) $(GCCFLAGS) isr.c -o isr.o
	@$(ASM) $(ASMFLAGS) isrs.s -o isrs.o
	@$(GCC) $(GCCFLAGS) kbd.c -o kbd.o
	@$(GCC) $(GCCFLAGS) mmu.c -o mmu.o
	@$(ASM) $(ASMFLAGS) loader.s -o loader.o
	@$(GCC) $(GCCFLAGS) pit.c -o pit.o
	@$(GCC) $(GCCFLAGS) rtc.c -o rtc.o
	@$(GCC) $(GCCFLAGS) screen.c -o screen.o
	@$(GCC) $(GCCFLAGS) syscall.c -o syscall.o

clean:
	@rm -f *.o

