M:=\
    $(DIR_SRC)/modules/arch/x86/platform/platform.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/platform/platform.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/platform/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
