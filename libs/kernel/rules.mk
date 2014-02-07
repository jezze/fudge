LIBKERNEL:=$(LIBS_PATH)/kernel/libkernel.a
LIBKERNEL_OBJ:=$(LIBS_PATH)/kernel/binary.o $(LIBS_PATH)/kernel/binary_elf.o $(LIBS_PATH)/kernel/container.o $(LIBS_PATH)/kernel/error.o $(LIBS_PATH)/kernel/kernel.o $(LIBS_PATH)/kernel/scheduler.o $(LIBS_PATH)/kernel/task.o $(LIBS_PATH)/kernel/vfs.o $(LIBS_PATH)/kernel/vfs_cpio.o $(LIBS_PATH)/kernel/vfs_tar.o

$(LIBKERNEL): $(LIBKERNEL_OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBKERNEL)
LIBS_OBJECTS+=$(LIBKERNEL_OBJ)
