M:=\
    $(DIR_SRC)/modules/arch/x86/usb/ehci.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/usb/ehci.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/usb/ehci.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/arch/x86/usb/ohci.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/usb/ohci.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/usb/ohci.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/arch/x86/usb/uhci.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/usb/uhci.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/usb/uhci.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
