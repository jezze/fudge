M:=\
    $(DIR_SRC)/modules/wm/wm.ko \

O:=\
    $(DIR_SRC)/modules/wm/main.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
