M:=\
    $(DIR_SRC)/modules/arch/x86/cirrus/cirrus.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/cirrus/cirrus.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/cirrus/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
