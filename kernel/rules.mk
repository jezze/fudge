include kernel/src/arch/rules.$(ARCH).mk

KERNEL+=kernel/fudge
KERNEL_OBJECTS+=kernel/src/binary.o
KERNEL_OBJECTS+=kernel/src/error.o
KERNEL_OBJECTS+=kernel/src/kernel.o
KERNEL_OBJECTS+=kernel/src/mmu.o
KERNEL_OBJECTS+=kernel/src/ramdisk.o
KERNEL_OBJECTS+=kernel/src/runtime.o
KERNEL_OBJECTS+=kernel/src/syscall.o
KERNEL_OBJECTS+=kernel/src/vfs.o

kernel/%.o: CCFLAGS+=-Ilib/include -Ikernel/include

kernel/%: LDFLAGS+=-Tkernel/src/arch/linker.$(ARCH).ld

$(KERNEL): $(KERNEL_OBJECTS) $(LIB)
	$(LD) $(LDFLAGS) -o $@ $^
