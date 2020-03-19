M:=\
    $(DIR_SRC)/modules/arch/x86/io/io.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/io/io.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/io/io.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
