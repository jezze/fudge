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
    $(DIR_SRC)/kernel/service.o \
    $(DIR_SRC)/kernel/service_cpio.o \
    $(DIR_SRC)/format/cpio.o \
    $(DIR_SRC)/elf/elf.o \
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_SRC)/kernel/$(ARCH)/rules.mk

$(BIN_KERNEL): $(OBJ_KERNEL)
	$(LD) -o $@ $(LDFLAGS) -Tplatform/$(PLATFORM)/linker.ld $^

BIN:=$(BIN) $(BIN_KERNEL)
OBJ:=$(OBJ) $(OBJ_KERNEL)
