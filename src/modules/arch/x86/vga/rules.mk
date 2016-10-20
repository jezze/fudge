M:=\
    $(DIR_SRC)/modules/arch/x86/vga/vga.ko.0 \

O:=\
    $(DIR_SRC)/modules/arch/x86/vga/main.o \
    $(DIR_SRC)/modules/arch/x86/vga/registers.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
