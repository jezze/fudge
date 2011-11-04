ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I../../../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -m32

.PHONY: all clean arch

all: arch

arch:
	@${GCC} ${GCCFLAGS} arch.c -o arch.o
	@${ASM} ${ASMFLAGS} calls.s -o calls.o
	@${ASM} ${ASMFLAGS} cpu.s -o cpu.o
	@${GCC} ${GCCFLAGS} fpu.c -o fpu.o
	@${ASM} ${ASMFLAGS} fpus.s -o fpus.o
	@${GCC} ${GCCFLAGS} gdt.c -o gdt.o
	@${ASM} ${ASMFLAGS} gdts.s -o gdts.o
	@${GCC} ${GCCFLAGS} idt.c -o idt.o
	@${ASM} ${ASMFLAGS} idts.s -o idts.o
	@${ASM} ${ASMFLAGS} init.s -o init.o
	@${GCC} ${GCCFLAGS} irq.c -o irq.o
	@${ASM} ${ASMFLAGS} irqs.s -o irqs.o
	@${GCC} ${GCCFLAGS} isr.c -o isr.o
	@${ASM} ${ASMFLAGS} isrs.s -o isrs.o
	@${GCC} ${GCCFLAGS} mmu.c -o mmu.o
	@${GCC} ${GCCFLAGS} mboot.c -o mboot.o
	@${GCC} ${GCCFLAGS} syscall.c -o syscall.o
	@${GCC} ${GCCFLAGS} tss.c -o tss.o
	@${ASM} ${ASMFLAGS} tsss.s -o tsss.o

clean:
	@rm -f *.o

