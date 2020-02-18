M:=\
    $(DIR_SRC)/modules/arch/x86/rtl8139/rtl8139.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/rtl8139/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
