BIN_KERNEL:=\
    $(SRC_PATH)/kernel/fudge \

OBJ_KERNEL:=\
    $(SRC_PATH)/kernel/abi.o \
    $(SRC_PATH)/kernel/binary.o \
    $(SRC_PATH)/kernel/binary_elf.o \
    $(SRC_PATH)/kernel/container.o \
    $(SRC_PATH)/kernel/kernel.o \
    $(SRC_PATH)/kernel/resource.o \
    $(SRC_PATH)/kernel/task.o \
    $(SRC_PATH)/kernel/vfs.o \
    $(SRC_PATH)/kernel/vfs_cpio.o \
    $(SRC_PATH)/fudge/ascii.o \
    $(SRC_PATH)/fudge/buffer.o \
    $(SRC_PATH)/fudge/ctrl.o \
    $(SRC_PATH)/fudge/list.o \
    $(SRC_PATH)/fudge/memory.o \
    $(SRC_PATH)/format/cpio.o \
    $(SRC_PATH)/elf/elf.o \

include $(SRC_PATH)/kernel/$(ARCH)/rules.mk

$(BIN_KERNEL): $(OBJ_KERNEL) $(OBJ_STD)
	$(LD) -o $@ $(LDFLAGS) -Tplatform/$(PLATFORM)/linker.ld $^

BIN:=$(BIN) $(BIN_KERNEL)
CLEAN:=$(CLEAN) $(BIN_KERNEL) $(OBJ_KERNEL)
