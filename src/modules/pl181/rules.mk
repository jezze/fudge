M:=\
    $(DIR_SRC)/modules/pl181/pl181.ko \

N:=\
    $(DIR_SRC)/modules/pl181/pl181.ko.map \

O:=\
    $(DIR_SRC)/modules/pl181/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
