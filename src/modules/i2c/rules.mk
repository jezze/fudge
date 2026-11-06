M:=\
    $(DIR_SRC)/modules/i2c/i2c.ko \

N:=\
    $(DIR_SRC)/modules/i2c/i2c.ko.map \

O:=\
    $(DIR_SRC)/modules/i2c/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
