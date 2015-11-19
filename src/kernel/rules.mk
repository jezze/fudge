BIN_KERNEL:=\
    $(DIR_SRC)/kernel/$(KERNEL) \

OBJ_KERNEL:=\
    $(DIR_SRC)/kernel/abi.o \
    $(DIR_SRC)/kernel/binary.o \
    $(DIR_SRC)/kernel/binary_elf.o \
    $(DIR_SRC)/kernel/container.o \
    $(DIR_SRC)/kernel/kernel.o \
    $(DIR_SRC)/kernel/resource.o \
    $(DIR_SRC)/kernel/task.o \
    $(DIR_SRC)/kernel/vfs.o \
    $(DIR_SRC)/kernel/vfs_cpio.o \
    $(DIR_SRC)/fudge/ascii.o \
    $(DIR_SRC)/fudge/buffer.o \
    $(DIR_SRC)/fudge/ctrl.o \
    $(DIR_SRC)/fudge/list.o \
    $(DIR_SRC)/fudge/memory.o \
    $(DIR_SRC)/format/cpio.o \
    $(DIR_SRC)/elf/elf.o \

include $(DIR_SRC)/kernel/$(ARCH)/rules.mk

$(BIN_KERNEL): $(OBJ_KERNEL) $(OBJ_STD)
	$(LD) -o $@ $(LDFLAGS) -Tplatform/$(PLATFORM)/linker.ld $^

BIN:=$(BIN) $(BIN_KERNEL)
OBJ:=$(OBJ) $(OBJ_KERNEL)
