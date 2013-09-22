LIBKERNEL:=libs/kernel/libkernel.a
LIBKERNEL_OBJ:=libs/kernel/binary.o libs/kernel/binary_elf.o libs/kernel/container.o libs/kernel/error.o libs/kernel/kernel.o libs/kernel/task.o libs/kernel/vfs.o libs/kernel/vfs_cpio.o libs/kernel/vfs_system.o libs/kernel/vfs_tar.o

include libs/$(ARCH)/kernel/rules.mk
include libs/$(ARCH)/$(LOADER)/rules.mk

$(LIBKERNEL): $(LIBKERNEL_OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBKERNEL)
LIBS_OBJECTS+=$(LIBKERNEL_OBJ)
