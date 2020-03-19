M:=\
    $(DIR_SRC)/modules/mtwist/mtwist.ko \

N:=\
    $(DIR_SRC)/modules/mtwist/mtwist.ko.map \

O:=\
    $(DIR_SRC)/modules/mtwist/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
