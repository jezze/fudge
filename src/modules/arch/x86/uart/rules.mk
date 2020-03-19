M:=\
    $(DIR_SRC)/modules/arch/x86/uart/uart.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/uart/uart.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/uart/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
