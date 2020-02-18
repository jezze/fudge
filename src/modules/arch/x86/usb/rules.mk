M:=\
    $(DIR_SRC)/modules/arch/x86/usb/ehci.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/usb/ehci.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/arch/x86/usb/ohci.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/usb/ohci.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/arch/x86/usb/uhci.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/usb/uhci.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
