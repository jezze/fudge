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

M:=\
    $(DIR_SRC)/modules/amiga/amiga-keyboard.ko \

N:=\
    $(DIR_SRC)/modules/amiga/amiga-keyboard.ko.map \

O:=\
    $(DIR_SRC)/modules/amiga/keyboard.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/amiga/amiga-mouse.ko \

N:=\
    $(DIR_SRC)/modules/amiga/amiga-mouse.ko.map \

O:=\
    $(DIR_SRC)/modules/amiga/mouse.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/amiga/amiga-video.ko \

N:=\
    $(DIR_SRC)/modules/amiga/amiga-video.ko.map \

O:=\
    $(DIR_SRC)/modules/amiga/video.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
