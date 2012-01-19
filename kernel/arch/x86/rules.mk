kernel-arch: ASM=nasm
kernel-arch: ASMFLAGS=-f elf
kernel-arch: GCCFLAGS+=-Iinclude

kernel-arch:
	@${GCC} ${GCCFLAGS} kernel/arch/x86/arch.c -o kernel/arch/x86/arch.o
	@${ASM} ${ASMFLAGS} kernel/arch/x86/cpu.s -o kernel/arch/x86/cpu.o
	@${GCC} ${GCCFLAGS} kernel/arch/x86/fpu.c -o kernel/arch/x86/fpu.o
	@${GCC} ${GCCFLAGS} kernel/arch/x86/gdt.c -o kernel/arch/x86/gdt.o
	@${GCC} ${GCCFLAGS} kernel/arch/x86/idt.c -o kernel/arch/x86/idt.o
	@${ASM} ${ASMFLAGS} kernel/arch/x86/init.s -o kernel/arch/x86/init.o
	@${ASM} ${ASMFLAGS} kernel/arch/x86/io.s -o kernel/arch/x86/io.o
	@${GCC} ${GCCFLAGS} kernel/arch/x86/irq.c -o kernel/arch/x86/irq.o
	@${ASM} ${ASMFLAGS} kernel/arch/x86/irqs.s -o kernel/arch/x86/irqs.o
	@${GCC} ${GCCFLAGS} kernel/arch/x86/isr.c -o kernel/arch/x86/isr.o
	@${ASM} ${ASMFLAGS} kernel/arch/x86/isrs.s -o kernel/arch/x86/isrs.o
	@${GCC} ${GCCFLAGS} kernel/arch/x86/mmu.c -o kernel/arch/x86/mmu.o
	@${GCC} ${GCCFLAGS} kernel/arch/x86/mboot.c -o kernel/arch/x86/mboot.o
	@${GCC} ${GCCFLAGS} kernel/arch/x86/syscall.c -o kernel/arch/x86/syscall.o
	@${GCC} ${GCCFLAGS} kernel/arch/x86/tss.c -o kernel/arch/x86/tss.o

kernel-arch-clean:
	@rm -f kernel/arch/x86/*.o

