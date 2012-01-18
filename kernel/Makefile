include ../rules.mk
include ../${ARCH}.mk
include arch/${ARCH}/rules.mk

kernel: GCCFLAGS+=-I../include
kernel-arm: LDFLAGS+=-Tarch/arm/linker.ld
kernel-x86: LDFLAGS+=-Tarch/x86/linker.ld

.PHONY: all clean kernel arch arch-clean

all: kernel kernel-${ARCH}

kernel: arch
	@${GCC} ${GCCFLAGS} elf.c -o elf.o
	@${GCC} ${GCCFLAGS} error.c -o error.o
	@${GCC} ${GCCFLAGS} event.c -o event.o
	@${GCC} ${GCCFLAGS} initrd.c -o initrd.o
	@${GCC} ${GCCFLAGS} irq.c -o irq.o
	@${GCC} ${GCCFLAGS} kernel.c -o kernel.o
	@${GCC} ${GCCFLAGS} log.c -o log.o
	@${GCC} ${GCCFLAGS} mmu.c -o mmu.o
	@${GCC} ${GCCFLAGS} modules.c -o modules.o
	@${GCC} ${GCCFLAGS} runtime.c -o runtime.o
	@${GCC} ${GCCFLAGS} symbol.c -o symbol.o
	@${GCC} ${GCCFLAGS} syscall.c -o syscall.o
	@${GCC} ${GCCFLAGS} vfs.c -o vfs.o

kernel-arm:
	@${LD} ${LDFLAGS} elf.o error.o event.o initrd.o irq.o kernel.o log.o mmu.o modules.o runtime.o symbol.o syscall.o vfs.o \
		arch/arm/arch.o \
		arch/arm/cpu.o \
		arch/arm/init.o \
		../lib/memory.o \
		../lib/string.o \
		-o fudge 

kernel-x86:
	@${LD} ${LDFLAGS} elf.o error.o event.o initrd.o irq.o kernel.o log.o mmu.o modules.o runtime.o symbol.o syscall.o vfs.o \
		arch/x86/acpi.o \
		arch/x86/arch.o \
		arch/x86/cpu.o \
		arch/x86/fpu.o \
		arch/x86/gdt.o \
		arch/x86/idt.o \
		arch/x86/init.o \
		arch/x86/io.o \
		arch/x86/irqs.o \
		arch/x86/isrs.o \
		arch/x86/irq.o \
		arch/x86/isr.o \
		arch/x86/mmu.o \
		arch/x86/mboot.o \
		arch/x86/syscall.o \
		arch/x86/tss.o \
		../lib/memory.o \
		../lib/string.o \
		-o fudge

clean: arch-clean
	@rm -f fudge
	@rm -f *.o

