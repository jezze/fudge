ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I../../../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -m32

.PHONY: all clean arch

all: arch

arch:
	@${GCC} ${GCCFLAGS} acpi.c -o acpi.o
	@${GCC} ${GCCFLAGS} arch.c -o arch.o
	@${ASM} ${ASMFLAGS} cpu.s -o cpu.o
	@${GCC} ${GCCFLAGS} fpu.c -o fpu.o
	@${GCC} ${GCCFLAGS} gdt.c -o gdt.o
	@${GCC} ${GCCFLAGS} idt.c -o idt.o
	@${ASM} ${ASMFLAGS} init.s -o init.o
	@${ASM} ${ASMFLAGS} io.s -o io.o
	@${GCC} ${GCCFLAGS} irq.c -o irq.o
	@${ASM} ${ASMFLAGS} irqs.s -o irqs.o
	@${GCC} ${GCCFLAGS} isr.c -o isr.o
	@${ASM} ${ASMFLAGS} isrs.s -o isrs.o
	@${GCC} ${GCCFLAGS} mmu.c -o mmu.o
	@${GCC} ${GCCFLAGS} mboot.c -o mboot.o
	@${GCC} ${GCCFLAGS} syscall.c -o syscall.o
	@${GCC} ${GCCFLAGS} tss.c -o tss.o

clean:
	@rm -f *.o

