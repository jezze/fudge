M:=\
    $(DIR_SRC)/modules/arch/x86/platform/platform.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/platform/main.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
