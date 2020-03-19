M:=\
    $(DIR_SRC)/modules/null/null.ko \

N:=\
    $(DIR_SRC)/modules/null/null.ko.map \

O:=\
    $(DIR_SRC)/modules/null/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
