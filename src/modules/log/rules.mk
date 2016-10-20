M:=\
    $(DIR_SRC)/modules/log/log.ko \

O:=\
    $(DIR_SRC)/modules/log/main.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
