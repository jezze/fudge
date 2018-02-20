M:=\
    $(DIR_SRC)/modules/arch/x86/gdb/gdb.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/gdb/main.o \
    $(DIR_SRC)/modules/arch/x86/gdb/i386-stub.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
