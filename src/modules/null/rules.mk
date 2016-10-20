M:=\
    $(DIR_SRC)/modules/null/null.ko \

O:=\
    $(DIR_SRC)/modules/null/main.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
