M:=\
    $(DIR_SRC)/modules/base/base.ko \

O:=\
    $(DIR_SRC)/modules/base/main.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
