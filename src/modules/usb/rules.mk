M:=\
    $(DIR_SRC)/modules/usb/usb.ko \

N:=\
    $(DIR_SRC)/modules/usb/usb.ko.map \

O:=\
    $(DIR_SRC)/modules/usb/bus.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/usb/usb-keyboard.ko \

N:=\
    $(DIR_SRC)/modules/usb/usb-keyboard.ko.map \

O:=\
    $(DIR_SRC)/modules/usb/keyboard.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/usb/usb-mouse.ko \

N:=\
    $(DIR_SRC)/modules/usb/usb-mouse.ko.map \

O:=\
    $(DIR_SRC)/modules/usb/mouse.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/usb/usb-block.ko \

N:=\
    $(DIR_SRC)/modules/usb/usb-block.ko.map \

O:=\
    $(DIR_SRC)/modules/usb/block.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
