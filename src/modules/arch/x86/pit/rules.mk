M:=\
    $(DIR_SRC)/modules/arch/x86/pit/pit.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/pit/pit.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/pit/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
