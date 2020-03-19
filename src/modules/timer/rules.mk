M:=\
    $(DIR_SRC)/modules/timer/timer.ko \

N:=\
    $(DIR_SRC)/modules/timer/timer.ko.map \

O:=\
    $(DIR_SRC)/modules/timer/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
