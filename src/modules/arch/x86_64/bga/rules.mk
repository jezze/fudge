M:=\
    $(DIR_SRC)/modules/arch/x86_64/bga/bga.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86_64/bga/bga.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86_64/bga/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
