M:=\
    $(DIR_SRC)/modules/arch/x86/smp/smp.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/smp/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
