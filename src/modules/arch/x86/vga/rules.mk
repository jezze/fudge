M:=\
    $(DIR_SRC)/modules/arch/x86/vga/vga.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/vga/vga.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/vga/main.o \
    $(DIR_SRC)/modules/arch/x86/vga/registers.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
