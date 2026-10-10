M:=\
    $(DIR_SRC)/modules/vga/vga.ko \

N:=\
    $(DIR_SRC)/modules/vga/vga.ko.map \

O:=\
    $(DIR_SRC)/modules/vga/main.o \
    $(DIR_SRC)/modules/vga/registers.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
