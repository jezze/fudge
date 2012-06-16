include kernel/arch/$(ARCH)/rules.mk
include kernel/vfs/rules.mk

KERNELOBJ+=kernel/error.o kernel/event.o kernel/irq.o kernel/kernel.o kernel/mmu.o kernel/modules.o kernel/ramdisk.o kernel/runtime.o kernel/syscall.o
CLEAN+=$(KERNELOBJ) kernel/fudge

kernel/%.o: CCFLAGS+=-Iinclude

kernel/%: LDFLAGS+=-Tkernel/arch/$(ARCH)/linker.ld

kernel/fudge: $(KERNELOBJ) lib/libfudge.a
	$(LD) $(LDFLAGS) -o $@ $^
