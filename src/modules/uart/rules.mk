M:=\
    $(DIR_SRC)/modules/uart/uart.ko \

N:=\
    $(DIR_SRC)/modules/uart/uart.ko.map \

O:=\
    $(DIR_SRC)/modules/uart/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
