LIB:=libs/kernel/libkernel.a
OBJ:=libs/kernel/binary.o
OBJ+=libs/kernel/binary_elf.o
OBJ+=libs/kernel/error.o
OBJ+=libs/kernel/kernel.o
OBJ+=libs/kernel/runtime.o
OBJ+=libs/kernel/syscall.o
OBJ+=libs/kernel/vfs.o
OBJ+=libs/kernel/vfs_tar.o

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBKERNEL:=$(LIB)
LIBS+=$(LIB)
LIBS_OBJECTS+=$(OBJ)
