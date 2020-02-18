M:=\
    $(DIR_SRC)/modules/arch/x86/ps2/ps2.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/ps2/bus.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/arch/x86/ps2/ps2-keyboard.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/ps2/keyboard.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/arch/x86/ps2/ps2-mouse.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/ps2/mouse.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
