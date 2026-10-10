M:=\
    $(DIR_SRC)/modules/pit/pit.ko \

N:=\
    $(DIR_SRC)/modules/pit/pit.ko.map \

O:=\
    $(DIR_SRC)/modules/pit/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
