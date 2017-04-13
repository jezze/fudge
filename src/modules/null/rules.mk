M:=\
    $(DIR_SRC)/modules/null/null.ko \

O:=\
    $(DIR_SRC)/modules/null/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
