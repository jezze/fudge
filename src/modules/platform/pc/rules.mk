M:=\
    $(DIR_SRC)/modules/platform/pc/platform.ko \

N:=\
    $(DIR_SRC)/modules/platform/pc/platform.ko.map \

O:=\
    $(DIR_SRC)/modules/platform/pc/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
