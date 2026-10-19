M:=\
    $(DIR_SRC)/modules/goldfish/goldfish.ko \

N:=\
    $(DIR_SRC)/modules/goldfish/goldfish.ko.map \

O:=\
    $(DIR_SRC)/modules/goldfish/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/goldfish/goldfish-tty.ko \

N:=\
    $(DIR_SRC)/modules/goldfish/goldfish-tty.ko.map \

O:=\
    $(DIR_SRC)/modules/goldfish/tty.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
