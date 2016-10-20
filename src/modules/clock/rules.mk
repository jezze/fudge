M:=\
    $(DIR_SRC)/modules/clock/clock.ko \

O:=\
    $(DIR_SRC)/modules/clock/main.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
