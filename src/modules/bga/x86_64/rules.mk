M:=\
    $(DIR_SRC)/modules/bga/x86_64/bga.ko \

N:=\
    $(DIR_SRC)/modules/bga/x86_64/bga.ko.map \

O:=\
    $(DIR_SRC)/modules/bga/x86_64/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
