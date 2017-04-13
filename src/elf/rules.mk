L:=\
    $(DIR_SRC)/elf/libelf.a \

O:=\
    $(DIR_SRC)/elf/libelf.o \

include $(DIR_MK)/lib.mk

B:=\
    $(DIR_SRC)/elf/elfload \

O:=\
    $(DIR_SRC)/elf/elfload.o \

L:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/elf/libelf.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/elf/elfunload \

O:=\
    $(DIR_SRC)/elf/elfunload.o \

L:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/elf/libelf.a \

include $(DIR_MK)/bin.mk
