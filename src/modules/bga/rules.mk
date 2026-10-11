M:=\
    $(DIR_SRC)/modules/bga/bga.ko \

N:=\
    $(DIR_SRC)/modules/bga/bga.ko.map \

O:=\
    $(DIR_SRC)/modules/bga/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
