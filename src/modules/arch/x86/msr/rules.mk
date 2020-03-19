M:=\
    $(DIR_SRC)/modules/arch/x86/msr/msr.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/msr/msr.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/msr/main.o \
    $(DIR_SRC)/modules/arch/x86/msr/msr.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
