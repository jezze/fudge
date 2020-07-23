M:=\
    $(DIR_SRC)/modules/arch/x86/pat/pat.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/pat/pat.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/pat/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
