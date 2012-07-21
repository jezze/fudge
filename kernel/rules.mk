include kernel/src/arch/rules.$(ARCH).mk

KERNEL=kernel/fudge

KERNELOBJ+=kernel/src/error.o kernel/src/event.o kernel/src/kernel.o kernel/src/mmu.o kernel/src/modules.o kernel/src/ramdisk.o kernel/src/runtime.o kernel/src/syscall.o
KERNELOBJ+=kernel/src/vfs/ramdisk.o kernel/src/vfs/sys.o

CLEAN+=$(KERNELOBJ) $(KERNEL)

kernel/%.o: CCFLAGS+=-Ilib/include -Ikernel/include

kernel/%: LDFLAGS+=-Tkernel/src/arch/linker.$(ARCH).ld

kernel/fudge: $(KERNELOBJ) $(LIB) 
	$(LD) $(LDFLAGS) -o $@ $^
