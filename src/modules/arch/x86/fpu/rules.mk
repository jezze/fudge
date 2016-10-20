M:=\
    $(DIR_SRC)/modules/arch/x86/fpu/fpu.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/fpu/main.o \
    $(DIR_SRC)/modules/arch/x86/fpu/fpu.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
