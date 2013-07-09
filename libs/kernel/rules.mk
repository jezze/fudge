LIBKERNEL:=libs/kernel/libkernel.a
LIBKERNEL_OBJECTS:=libs/kernel/binary.o libs/kernel/binary_elf.o libs/kernel/container.o libs/kernel/error.o libs/kernel/kernel.o libs/kernel/task.o libs/kernel/vfs.o libs/kernel/vfs_cpio.o libs/kernel/vfs_tar.o

$(LIBKERNEL): $(LIBKERNEL_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBKERNEL)
LIBS_OBJECTS+=$(LIBKERNEL_OBJECTS)
