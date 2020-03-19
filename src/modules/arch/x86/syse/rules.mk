M:=\
    $(DIR_SRC)/modules/arch/x86/syse/syse.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/syse/syse.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/syse/main.o \
    $(DIR_SRC)/modules/arch/x86/syse/syse.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
