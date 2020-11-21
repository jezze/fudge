M:=\
    $(DIR_SRC)/modules/arch/x86/rtl8139/rtl8139.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/rtl8139/rtl8139.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/rtl8139/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/net/net.a \

include $(DIR_MK)/kmod.mk
