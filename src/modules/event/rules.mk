M:=\
    $(DIR_SRC)/modules/event/event.ko \

N:=\
    $(DIR_SRC)/modules/event/event.ko.map \

O:=\
    $(DIR_SRC)/modules/event/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
