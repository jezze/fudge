M:=\
    $(DIR_SRC)/modules/arch/x86/ide/ide.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/ide/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
