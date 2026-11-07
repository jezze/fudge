M:=\
    $(DIR_SRC)/modules/i2c/i2c.ko \

N:=\
    $(DIR_SRC)/modules/i2c/i2c.ko.map \

O:=\
    $(DIR_SRC)/modules/i2c/bus.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/i2c/i2c-hid.0.ko \

N:=\
    $(DIR_SRC)/modules/i2c/i2c-hid.0.ko.map \

O:=\
    $(DIR_SRC)/modules/i2c/hid.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/i2c/i2c-hid.1.ko \

N:=\
    $(DIR_SRC)/modules/i2c/i2c-hid.1.ko.map \

O:=\
    $(DIR_SRC)/modules/i2c/hid.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
