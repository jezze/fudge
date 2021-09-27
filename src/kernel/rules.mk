B:=\
    $(DIR_SRC)/kernel/$(KERNEL) \

N:=\
    $(DIR_SRC)/kernel/$(KERNEL).map \

O:=\
    $(DIR_SRC)/kernel/abi.o \
    $(DIR_SRC)/kernel/binary.o \
    $(DIR_SRC)/kernel/core.o \
    $(DIR_SRC)/kernel/debug.o \
    $(DIR_SRC)/kernel/descriptor.o \
    $(DIR_SRC)/kernel/kernel.o \
    $(DIR_SRC)/kernel/link.o \
    $(DIR_SRC)/kernel/mailbox.o \
    $(DIR_SRC)/kernel/resource.o \
    $(DIR_SRC)/kernel/task.o \
    $(DIR_SRC)/kernel/service.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_SRC)/kernel/$(ARCH)/rules.mk
include $(DIR_MK)/kbin.mk
