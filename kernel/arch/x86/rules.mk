arch: ASM=nasm
arch: ASMFLAGS=-f elf

arch:
	@${GCC} ${GCCFLAGS} arch/x86/acpi.c -o arch/x86/acpi.o
	@${GCC} ${GCCFLAGS} arch/x86/arch.c -o arch/x86/arch.o
	@${ASM} ${ASMFLAGS} arch/x86/cpu.s -o arch/x86/cpu.o
	@${GCC} ${GCCFLAGS} arch/x86/fpu.c -o arch/x86/fpu.o
	@${GCC} ${GCCFLAGS} arch/x86/gdt.c -o arch/x86/gdt.o
	@${GCC} ${GCCFLAGS} arch/x86/idt.c -o arch/x86/idt.o
	@${ASM} ${ASMFLAGS} arch/x86/init.s -o arch/x86/init.o
	@${ASM} ${ASMFLAGS} arch/x86/io.s -o arch/x86/io.o
	@${GCC} ${GCCFLAGS} arch/x86/irq.c -o arch/x86/irq.o
	@${ASM} ${ASMFLAGS} arch/x86/irqs.s -o arch/x86/irqs.o
	@${GCC} ${GCCFLAGS} arch/x86/isr.c -o arch/x86/isr.o
	@${ASM} ${ASMFLAGS} arch/x86/isrs.s -o arch/x86/isrs.o
	@${GCC} ${GCCFLAGS} arch/x86/mmu.c -o arch/x86/mmu.o
	@${GCC} ${GCCFLAGS} arch/x86/mboot.c -o arch/x86/mboot.o
	@${GCC} ${GCCFLAGS} arch/x86/syscall.c -o arch/x86/syscall.o
	@${GCC} ${GCCFLAGS} arch/x86/tss.c -o arch/x86/tss.o

arch-clean:
	@rm -f arch/x86/*.o

