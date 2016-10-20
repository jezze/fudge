M:=\
    $(DIR_SRC)/modules/console/console.ko \

O:=\
    $(DIR_SRC)/modules/console/main.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
