M:=\
    $(DIR_SRC)/modules/console/console.ko \

N:=\
    $(DIR_SRC)/modules/console/console.ko.map \

O:=\
    $(DIR_SRC)/modules/console/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
