include kernel/src/arch/$(ARCH).mk

KERNELOBJ+=kernel/src/error.o kernel/src/event.o kernel/src/kernel.o kernel/src/mmu.o kernel/src/modules.o kernel/src/ramdisk.o kernel/src/runtime.o kernel/src/syscall.o
KERNELOBJ+=kernel/src/vfs/ramdisk.o kernel/src/vfs/root.o kernel/src/vfs/sys.o
CLEAN+=$(KERNELOBJ) kernel/fudge

kernel/%.o: CCFLAGS+=-Ilib/include -Ikernel/include

kernel/%: LDFLAGS+=-Tkernel/src/arch/$(ARCH).ld

kernel/fudge: $(KERNELOBJ) lib/libfudge.a
	$(LD) $(LDFLAGS) -o $@ $^
