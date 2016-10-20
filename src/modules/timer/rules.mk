M:=\
    $(DIR_SRC)/modules/timer/timer.ko \

O:=\
    $(DIR_SRC)/modules/timer/main.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
