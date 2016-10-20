M:=\
    $(DIR_SRC)/modules/keyboard/keyboard.ko \

O:=\
    $(DIR_SRC)/modules/keyboard/main.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
