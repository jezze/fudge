M:=\
    $(DIR_SRC)/modules/usb/ehci.ko \

N:=\
    $(DIR_SRC)/modules/usb/ehci.ko.map \

O:=\
    $(DIR_SRC)/modules/usb/ehci.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/usb/ohci.ko \

N:=\
    $(DIR_SRC)/modules/usb/ohci.ko.map \

O:=\
    $(DIR_SRC)/modules/usb/ohci.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/usb/uhci.ko \

N:=\
    $(DIR_SRC)/modules/usb/uhci.ko.map \

O:=\
    $(DIR_SRC)/modules/usb/uhci.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
