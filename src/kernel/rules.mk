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

include $(SRC_PATH)/kernel/$(ARCH)/rules.mk

$(BIN_KERNEL): $(OBJ_KERNEL) $(KERNEL_DEPS)
	$(LD) -o $@ $(LDFLAGS) -Tplatform/$(PLATFORM)/linker.ld $^

BIN:=$(BIN) $(BIN_KERNEL)
CLEAN:=$(CLEAN) $(BIN_KERNEL) $(OBJ_KERNEL)
