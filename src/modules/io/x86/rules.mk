M:=\
    $(DIR_SRC)/modules/io/x86/io.ko \

N:=\
    $(DIR_SRC)/modules/io/x86/io.ko.map \

O:=\
    $(DIR_SRC)/modules/io/x86/io.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
