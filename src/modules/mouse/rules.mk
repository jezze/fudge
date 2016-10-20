M:=\
    $(DIR_SRC)/modules/mouse/mouse.ko \

O:=\
    $(DIR_SRC)/modules/mouse/main.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
