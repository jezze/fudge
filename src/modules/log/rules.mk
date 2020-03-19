M:=\
    $(DIR_SRC)/modules/log/log.ko \

N:=\
    $(DIR_SRC)/modules/log/log.ko.map \

O:=\
    $(DIR_SRC)/modules/log/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
