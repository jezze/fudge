M:=\
    $(DIR_SRC)/modules/amiga/amiga.ko \

N:=\
    $(DIR_SRC)/modules/amiga/amiga.ko.map \

O:=\
    $(DIR_SRC)/modules/amiga/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/amiga/amiga-timer.ko \

N:=\
    $(DIR_SRC)/modules/amiga/amiga-timer.ko.map \

O:=\
    $(DIR_SRC)/modules/amiga/timer.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/amiga/amiga-serial.ko \

N:=\
    $(DIR_SRC)/modules/amiga/amiga-serial.ko.map \

O:=\
    $(DIR_SRC)/modules/amiga/serial.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
