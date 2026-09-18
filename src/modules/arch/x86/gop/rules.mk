M:=\
    $(DIR_SRC)/modules/arch/x86/gop/gop.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/gop/gop.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/gop/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
