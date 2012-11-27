KERNEL+=kernel/fudge

KERNEL_OBJECTS+=kernel/src/binary.o
KERNEL_OBJECTS+=kernel/src/error.o
KERNEL_OBJECTS+=kernel/src/kernel.o
KERNEL_OBJECTS+=kernel/src/ramdisk.o
KERNEL_OBJECTS+=kernel/src/runtime.o
KERNEL_OBJECTS+=kernel/src/syscall.o
KERNEL_OBJECTS+=kernel/src/vfs.o
KERNEL_OBJECTS+=lib/src/elf.o
KERNEL_OBJECTS+=lib/src/memory.o
KERNEL_OBJECTS+=lib/src/posix.o
KERNEL_OBJECTS+=lib/src/string.o

$(KERNEL): $(LIB) $(KERNEL_OBJECTS)
	$(LD) $(LDFLAGS) -Tkernel/linker.$(ARCH).ld -o $@ $^ $(EXTRA)

kernel/%.o: kernel/%.c
	$(CC) -c $(CFLAGS) -Ilib/include -Ikernel/include -o $@ $<
