M:=\
    $(DIR_SRC)/modules/system/system.ko \

N:=\
    $(DIR_SRC)/modules/system/system.ko.map \

O:=\
    $(DIR_SRC)/modules/system/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
