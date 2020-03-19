M:=\
    $(DIR_SRC)/modules/arch/x86/gdb/gdb.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/gdb/gdb.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/gdb/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
