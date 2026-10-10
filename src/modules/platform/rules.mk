M:=\
    $(DIR_SRC)/modules/platform/platform.ko \

N:=\
    $(DIR_SRC)/modules/platform/platform.ko.map \

O:=\
    $(DIR_SRC)/modules/platform/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
