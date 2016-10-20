M:=\
    $(DIR_SRC)/modules/event/event.ko \

O:=\
    $(DIR_SRC)/modules/event/main.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
