M:=\
    $(DIR_SRC)/modules/ramfb/ramfb.ko \

N:=\
    $(DIR_SRC)/modules/ramfb/ramfb.ko.map \

O:=\
    $(DIR_SRC)/modules/ramfb/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
