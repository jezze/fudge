M:=\
    $(DIR_SRC)/modules/ac97/ac97.ko \

N:=\
    $(DIR_SRC)/modules/ac97/ac97.ko.map \

O:=\
    $(DIR_SRC)/modules/ac97/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
