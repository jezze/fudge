M:=\
    $(DIR_SRC)/modules/ide/ide.ko \

N:=\
    $(DIR_SRC)/modules/ide/ide.ko.map \

O:=\
    $(DIR_SRC)/modules/ide/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
