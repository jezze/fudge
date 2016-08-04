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
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/format/format.a \

include $(DIR_SRC)/kernel/$(ARCH)/rules.mk

$(BIN_KERNEL): LDFLAGS+=-Tplatform/$(PLATFORM)/linker.ld
$(BIN_KERNEL): $(OBJ_KERNEL)
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

BIN:=$(BIN) $(BIN_KERNEL)
OBJ:=$(OBJ) $(OBJ_KERNEL)
