include kernel/src/arch/rules.$(ARCH).mk

KERNEL+=kernel/fudge
KERNEL_OBJECTS+=kernel/src/binary.o kernel/src/error.o kernel/src/event.o kernel/src/kernel.o kernel/src/mmu.o kernel/src/modules.o kernel/src/ramdisk.o kernel/src/runtime.o kernel/src/syscall.o
KERNEL_OBJECTS+=kernel/src/vfs/ramdisk.o kernel/src/vfs/sys.o

kernel/%.o: CCFLAGS+=-Ilib/include -Ikernel/include

kernel/%: LDFLAGS+=-Tkernel/src/arch/linker.$(ARCH).ld

$(KERNEL): $(KERNEL_OBJECTS) $(LIB) 
	$(LD) $(LDFLAGS) -o $@ $^
