M:=\
    $(DIR_SRC)/modules/ns16550/ns16550.ko \

N:=\
    $(DIR_SRC)/modules/ns16550/ns16550.ko.map \

O:=\
    $(DIR_SRC)/modules/ns16550/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
