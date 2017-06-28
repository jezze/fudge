M:=\
    $(DIR_SRC)/modules/arch/x86/io/io.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/io/io.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
