include kernel/arch/${ARCH}/rules.mk
include kernel/vfs/rules.mk

kernel: GCCFLAGS+=-Iinclude
kernel: LDFLAGS+=-Tkernel/arch/${ARCH}/linker.ld

kernel: kernel-arch vfs
	@${GCC} ${GCCFLAGS} kernel/error.c -o kernel/error.o
	@${GCC} ${GCCFLAGS} kernel/event.c -o kernel/event.o
	@${GCC} ${GCCFLAGS} kernel/ramdisk.c -o kernel/ramdisk.o
	@${GCC} ${GCCFLAGS} kernel/irq.c -o kernel/irq.o
	@${GCC} ${GCCFLAGS} kernel/kernel.c -o kernel/kernel.o
	@${GCC} ${GCCFLAGS} kernel/log.c -o kernel/log.o
	@${GCC} ${GCCFLAGS} kernel/mmu.c -o kernel/mmu.o
	@${GCC} ${GCCFLAGS} kernel/modules.c -o kernel/modules.o
	@${GCC} ${GCCFLAGS} kernel/runtime.c -o kernel/runtime.o
	@${GCC} ${GCCFLAGS} kernel/syscall.c -o kernel/syscall.o
	@${LD} ${LDFLAGS} kernel/*.o kernel/arch/${ARCH}/*.o kernel/vfs/*.o lib/libfudge.a -o kernel/fudge

kernel-clean: kernel-arch-clean vfs-clean
	@rm -f kernel/fudge
	@rm -f kernel/*.o

