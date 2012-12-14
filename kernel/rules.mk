KERNEL=kernel/fudge

KERNEL_OBJECTS+=kernel/binary.o
KERNEL_OBJECTS+=kernel/error.o
KERNEL_OBJECTS+=kernel/kernel.o
KERNEL_OBJECTS+=kernel/ramdisk.o
KERNEL_OBJECTS+=kernel/runtime.o
KERNEL_OBJECTS+=kernel/syscall.o
KERNEL_OBJECTS+=kernel/vfs.o

include kernel/arch/$(ARCH)/rules.mk

$(KERNEL): $(KERNEL_OBJECTS) $(LIBFUDGE) $(LIBMBOOT)
	$(LD) $(LDFLAGS) -Tlibs/mboot/linker.ld -o $@ $^

kernel/%.o: kernel/%.c
	$(CC) -c $(CFLAGS) -Ilibs -o $@ $<
