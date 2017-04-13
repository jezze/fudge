M:=\
    $(DIR_SRC)/modules/mtwist/mtwist.ko \

O:=\
    $(DIR_SRC)/modules/mtwist/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
