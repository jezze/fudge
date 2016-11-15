L:=\
    $(DIR_SRC)/abi/abi.a \

O:=\
    $(DIR_SRC)/abi/file.o \

include $(DIR_SRC)/abi/$(ARCH)/rules.mk
include $(DIR_MK)/lib.mk

L:=\
    $(DIR_SRC)/abi/qabi.a \

O:=\
    $(DIR_SRC)/abi/file.o \

include $(DIR_SRC)/abi/$(ARCH)/qrules.mk
include $(DIR_MK)/lib.mk
