LIB:=libs/kernel/libkernel.a
OBJ:=libs/kernel/binary.o libs/kernel/binary_elf.o libs/kernel/error.o libs/kernel/kernel.o libs/kernel/runtime.o libs/kernel/syscall.o libs/kernel/vfs.o libs/kernel/vfs_ramdisk.o

include libs/kernel/arch/$(ARCH)/rules.mk

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBKERNEL:=$(LIB)
LIBS+=$(LIB)
LIBS_OBJECTS+=$(OBJ)
