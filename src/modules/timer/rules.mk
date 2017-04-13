M:=\
    $(DIR_SRC)/modules/timer/timer.ko \

O:=\
    $(DIR_SRC)/modules/timer/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
