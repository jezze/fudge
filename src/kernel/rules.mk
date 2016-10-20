B:=\
    $(DIR_SRC)/kernel/$(KERNEL) \

O:=\
    $(DIR_SRC)/kernel/abi.o \
    $(DIR_SRC)/kernel/binary.o \
    $(DIR_SRC)/kernel/binary_elf.o \
    $(DIR_SRC)/kernel/container.o \
    $(DIR_SRC)/kernel/debug.o \
    $(DIR_SRC)/kernel/kernel.o \
    $(DIR_SRC)/kernel/resource.o \
    $(DIR_SRC)/kernel/task.o \
    $(DIR_SRC)/kernel/service.o \
    $(DIR_SRC)/kernel/service_cpio.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/format/format.a \

include $(DIR_SRC)/kernel/$(ARCH)/rules.mk

$(B): LDFLAGS+=-Tplatform/$(PLATFORM)/linker.ld

include $(DIR_MK)/bin.mk
