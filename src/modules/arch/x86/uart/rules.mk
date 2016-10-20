M:=\
    $(DIR_SRC)/modules/arch/x86/uart/uart.ko.0 \

O:=\
    $(DIR_SRC)/modules/arch/x86/uart/main.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
