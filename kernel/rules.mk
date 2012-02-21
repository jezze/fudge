include kernel/arch/${ARCH}/rules.mk

kernel: GCCFLAGS+=-Iinclude
kernel: LDFLAGS+=-Tkernel/arch/${ARCH}/linker.ld

kernel: kernel-arch
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
	@${LD} ${LDFLAGS} kernel/*.o kernel/arch/${ARCH}/*.o lib/memory.o lib/string.o -o kernel/fudge

kernel-clean: kernel-arch-clean
	@rm -f kernel/fudge
	@rm -f kernel/*.o

