M:=\
    $(DIR_SRC)/modules/info/info.ko \

N:=\
    $(DIR_SRC)/modules/info/info.ko.map \

O:=\
    $(DIR_SRC)/modules/info/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
