M:=\
    $(DIR_SRC)/modules/pl050/pl050.ko \

N:=\
    $(DIR_SRC)/modules/pl050/pl050.ko.map \

O:=\
    $(DIR_SRC)/modules/pl050/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/pl050/pl050-keyboard.ko \

N:=\
    $(DIR_SRC)/modules/pl050/pl050-keyboard.ko.map \

O:=\
    $(DIR_SRC)/modules/pl050/keyboard.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/pl050/pl050-mouse.ko \

N:=\
    $(DIR_SRC)/modules/pl050/pl050-mouse.ko.map \

O:=\
    $(DIR_SRC)/modules/pl050/mouse.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
