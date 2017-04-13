M:=\
    $(DIR_SRC)/modules/console/console.ko \

O:=\
    $(DIR_SRC)/modules/console/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
