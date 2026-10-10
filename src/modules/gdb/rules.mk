M:=\
    $(DIR_SRC)/modules/gdb/gdb.ko \

N:=\
    $(DIR_SRC)/modules/gdb/gdb.ko.map \

O:=\
    $(DIR_SRC)/modules/gdb/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
