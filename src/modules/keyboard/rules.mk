M:=\
    $(DIR_SRC)/modules/keyboard/keyboard.ko \

N:=\
    $(DIR_SRC)/modules/keyboard/keyboard.ko.map \

O:=\
    $(DIR_SRC)/modules/keyboard/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
