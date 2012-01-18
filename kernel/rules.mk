include kernel/arch/${ARCH}/rules.mk

kernel: GCCFLAGS+=-Iinclude
kernel-arm: LDFLAGS+=-Tkernel/arch/arm/linker.ld
kernel-x86: LDFLAGS+=-Tkernel/arch/x86/linker.ld

kernel: arch
	@${GCC} ${GCCFLAGS} kernel/elf.c -o kernel/elf.o
	@${GCC} ${GCCFLAGS} kernel/error.c -o kernel/error.o
	@${GCC} ${GCCFLAGS} kernel/event.c -o kernel/event.o
	@${GCC} ${GCCFLAGS} kernel/initrd.c -o kernel/initrd.o
	@${GCC} ${GCCFLAGS} kernel/irq.c -o kernel/irq.o
	@${GCC} ${GCCFLAGS} kernel/kernel.c -o kernel/kernel.o
	@${GCC} ${GCCFLAGS} kernel/log.c -o kernel/log.o
	@${GCC} ${GCCFLAGS} kernel/mmu.c -o kernel/mmu.o
	@${GCC} ${GCCFLAGS} kernel/modules.c -o kernel/modules.o
	@${GCC} ${GCCFLAGS} kernel/runtime.c -o kernel/runtime.o
	@${GCC} ${GCCFLAGS} kernel/symbol.c -o kernel/symbol.o
	@${GCC} ${GCCFLAGS} kernel/syscall.c -o kernel/syscall.o
	@${GCC} ${GCCFLAGS} kernel/vfs.c -o kernel/vfs.o

kernel-arm:
	@${LD} ${LDFLAGS} kernel/elf.o kernel/error.o kernel/event.o kernel/initrd.o kernel/irq.o kernel/kernel.o kernel/log.o kernel/mmu.o kernel/modules.o kernel/runtime.o kernel/symbol.o kernel/syscall.o kernel/vfs.o \
		kernel/arch/arm/arch.o \
		kernel/arch/arm/cpu.o \
		kernel/arch/arm/init.o \
		lib/memory.o \
		lib/string.o \
		-o kernel/fudge 

kernel-x86:
	@${LD} ${LDFLAGS} kernel/elf.o kernel/error.o kernel/event.o kernel/initrd.o kernel/irq.o kernel/kernel.o kernel/log.o kernel/mmu.o kernel/modules.o kernel/runtime.o kernel/symbol.o kernel/syscall.o kernel/vfs.o \
		kernel/arch/x86/acpi.o \
		kernel/arch/x86/arch.o \
		kernel/arch/x86/cpu.o \
		kernel/arch/x86/fpu.o \
		kernel/arch/x86/gdt.o \
		kernel/arch/x86/idt.o \
		kernel/arch/x86/init.o \
		kernel/arch/x86/io.o \
		kernel/arch/x86/irqs.o \
		kernel/arch/x86/isrs.o \
		kernel/arch/x86/irq.o \
		kernel/arch/x86/isr.o \
		kernel/arch/x86/mmu.o \
		kernel/arch/x86/mboot.o \
		kernel/arch/x86/syscall.o \
		kernel/arch/x86/tss.o \
		lib/memory.o \
		lib/string.o \
		-o kernel/fudge

kernel-clean: arch-clean
	@rm -f kernel/fudge
	@rm -f kernel/*.o

