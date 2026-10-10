M:=\
    $(DIR_SRC)/modules/atapi/atapi.ko \

N:=\
    $(DIR_SRC)/modules/atapi/atapi.ko.map \

O:=\
    $(DIR_SRC)/modules/atapi/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
