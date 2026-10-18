M:=\
    $(DIR_SRC)/modules/pl031/pl031.ko \

N:=\
    $(DIR_SRC)/modules/pl031/pl031.ko.map \

O:=\
    $(DIR_SRC)/modules/pl031/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
