M:=\
    $(DIR_SRC)/modules/platform/amiga/platform.ko \

N:=\
    $(DIR_SRC)/modules/platform/amiga/platform.ko.map \

O:=\
    $(DIR_SRC)/modules/platform/amiga/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/platform/amiga/amiga-timer.ko \

N:=\
    $(DIR_SRC)/modules/platform/amiga/amiga-timer.ko.map \

O:=\
    $(DIR_SRC)/modules/platform/amiga/timer.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/platform/amiga/amiga-serial.ko \

N:=\
    $(DIR_SRC)/modules/platform/amiga/amiga-serial.ko.map \

O:=\
    $(DIR_SRC)/modules/platform/amiga/serial.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/platform/amiga/amiga-keyboard.ko \

N:=\
    $(DIR_SRC)/modules/platform/amiga/amiga-keyboard.ko.map \

O:=\
    $(DIR_SRC)/modules/platform/amiga/keyboard.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/platform/amiga/amiga-mouse.ko \

N:=\
    $(DIR_SRC)/modules/platform/amiga/amiga-mouse.ko.map \

O:=\
    $(DIR_SRC)/modules/platform/amiga/mouse.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/platform/amiga/amiga-video.ko \

N:=\
    $(DIR_SRC)/modules/platform/amiga/amiga-video.ko.map \

O:=\
    $(DIR_SRC)/modules/platform/amiga/video.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
