M:=\
    $(DIR_SRC)/modules/arch/x86/64bit/64bit.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/64bit/main.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
