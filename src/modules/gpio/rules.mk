M:=\
    $(DIR_SRC)/modules/gpio/gpio.ko \

N:=\
    $(DIR_SRC)/modules/gpio/gpio.ko.map \

O:=\
    $(DIR_SRC)/modules/gpio/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
