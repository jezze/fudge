include kernel/rules.$(ARCH).mk

KERNEL+=kernel/fudge

KERNEL_OBJECTS+=kernel/src/binary.o
KERNEL_OBJECTS+=kernel/src/error.o
KERNEL_OBJECTS+=kernel/src/kernel.o
KERNEL_OBJECTS+=kernel/src/ramdisk.o
KERNEL_OBJECTS+=kernel/src/runtime.o
KERNEL_OBJECTS+=kernel/src/syscall.o
KERNEL_OBJECTS+=kernel/src/vfs.o
KERNEL_OBJECTS+=libs/fudge/elf.o
KERNEL_OBJECTS+=libs/fudge/memory.o
KERNEL_OBJECTS+=libs/fudge/posix.o
KERNEL_OBJECTS+=libs/fudge/string.o

$(KERNEL): $(LIBFUDGE) $(LIBMBOOT) $(KERNEL_OBJECTS)
	$(LD) $(LDFLAGS) -Tlibs/mboot/linker.ld -o $@ $^ $(EXTRA)

kernel/%.o: kernel/%.c
	$(CC) -c $(CFLAGS) -Ilibs -Ikernel/include -o $@ $<
