LIBKERNEL=libs/kernel/libkernel.a

LIBKERNEL_OBJECTS+=libs/kernel/binary.o
LIBKERNEL_OBJECTS+=libs/kernel/error.o
LIBKERNEL_OBJECTS+=libs/kernel/kernel.o
LIBKERNEL_OBJECTS+=libs/kernel/ramdisk.o
LIBKERNEL_OBJECTS+=libs/kernel/runtime.o
LIBKERNEL_OBJECTS+=libs/kernel/syscall.o
LIBKERNEL_OBJECTS+=libs/kernel/vfs.o

LIBS+=$(LIBKERNEL)
LIBS_OBJECTS+=$(LIBKERNEL_OBJECTS)

$(LIBKERNEL): $(LIBKERNEL_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

libs/kernel/%.o: libs/kernel/%.c
	$(CC) -c $(CFLAGS) -Ilibs -o $@ $<
