M:=\
    $(DIR_SRC)/modules/arch/x86/syse/syse.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/syse/main.o \
    $(DIR_SRC)/modules/arch/x86/syse/syse.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
