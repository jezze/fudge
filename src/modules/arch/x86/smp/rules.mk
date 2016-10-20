M:=\
    $(DIR_SRC)/modules/arch/x86/smp/smp.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/smp/main.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
