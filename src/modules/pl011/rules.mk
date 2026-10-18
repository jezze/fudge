M:=\
    $(DIR_SRC)/modules/pl011/pl011.ko \

N:=\
    $(DIR_SRC)/modules/pl011/pl011.ko.map \

O:=\
    $(DIR_SRC)/modules/pl011/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
