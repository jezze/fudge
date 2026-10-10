M:=\
    $(DIR_SRC)/modules/gop/x86/gop.ko \

N:=\
    $(DIR_SRC)/modules/gop/x86/gop.ko.map \

O:=\
    $(DIR_SRC)/modules/gop/x86/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
