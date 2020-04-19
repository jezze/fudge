M:=\
    $(DIR_SRC)/modules/arch/x86/vbe/vbe.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/vbe/vbe.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/vbe/main.o \
    $(DIR_SRC)/modules/arch/x86/vbe/videomode.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
