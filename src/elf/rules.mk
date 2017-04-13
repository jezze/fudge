B:=\
    $(DIR_SRC)/elf/elfload \

O:=\
    $(DIR_SRC)/elf/elfload.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/elf/elf.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/elf/elfunload \

O:=\
    $(DIR_SRC)/elf/elfunload.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/elf/elf.a \

include $(DIR_MK)/bin.mk
