L:=\
    $(DIR_LIB)/abi/abi.a \

O:=\
    $(DIR_LIB)/abi/channel.o \
    $(DIR_LIB)/abi/call.o \
    $(DIR_LIB)/abi/crt0.o \
    $(DIR_LIB)/abi/job.o \
    $(DIR_LIB)/abi/option.o \

include $(DIR_LIB)/abi/$(ARCH)/rules.mk
include $(DIR_MK)/lib.mk

ifeq ($(TARGET),i386-unknown-elf)
L:=\
    $(DIR_LIB)/abi/qabi.a \

O:=\
    $(DIR_LIB)/abi/channel.o \
    $(DIR_LIB)/abi/call.o \
    $(DIR_LIB)/abi/crt0.o \
    $(DIR_LIB)/abi/job.o \
    $(DIR_LIB)/abi/option.o \

include $(DIR_LIB)/abi/$(ARCH)/qrules.mk
include $(DIR_MK)/lib.mk
endif
