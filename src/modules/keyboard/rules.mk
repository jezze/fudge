M:=\
    $(DIR_SRC)/modules/keyboard/keyboard.ko \

O:=\
    $(DIR_SRC)/modules/keyboard/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
