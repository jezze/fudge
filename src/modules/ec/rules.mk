M:=\
    $(DIR_SRC)/modules/ec/ec.ko \

N:=\
    $(DIR_SRC)/modules/ec/ec.ko.map \

O:=\
    $(DIR_SRC)/modules/ec/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
