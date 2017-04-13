M:=\
    $(DIR_SRC)/modules/arch/x86/ps2/ps2.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/ps2/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
