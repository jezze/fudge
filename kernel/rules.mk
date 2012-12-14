include kernel/arch/$(ARCH)/rules.mk

KERNEL+=kernel/fudge

KERNEL_OBJECTS+=kernel/binary.o
KERNEL_OBJECTS+=kernel/error.o
KERNEL_OBJECTS+=kernel/kernel.o
KERNEL_OBJECTS+=kernel/ramdisk.o
KERNEL_OBJECTS+=kernel/runtime.o
KERNEL_OBJECTS+=kernel/syscall.o
KERNEL_OBJECTS+=kernel/vfs.o
KERNEL_OBJECTS+=libs/fudge/elf.o
KERNEL_OBJECTS+=libs/fudge/memory.o
KERNEL_OBJECTS+=libs/fudge/posix.o
KERNEL_OBJECTS+=libs/fudge/string.o

$(KERNEL): $(LIBFUDGE) $(LIBMBOOT) $(KERNEL_OBJECTS)
	$(LD) $(LDFLAGS) -Tlibs/mboot/linker.ld -o $@ $^ $(EXTRA)

kernel/%.o: kernel/%.c
	$(CC) -c $(CFLAGS) -Ilibs -o $@ $<
