M:=\
    $(DIR_SRC)/modules/mouse/mouse.ko \

N:=\
    $(DIR_SRC)/modules/mouse/mouse.ko.map \

O:=\
    $(DIR_SRC)/modules/mouse/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
