B:=\
    $(DIR_SRC)/elf/elfload \

O:=\
    $(DIR_SRC)/elf/elfload.o \

D:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/format/format.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/elf/elfunload \

O:=\
    $(DIR_SRC)/elf/elfunload.o \

D:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/format/format.a \

include $(DIR_MK)/bin.mk
