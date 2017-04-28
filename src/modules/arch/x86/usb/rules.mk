M:=\
    $(DIR_SRC)/modules/arch/x86/usb/ehci.ko.0 \

O:=\
    $(DIR_SRC)/modules/arch/x86/usb/ehci.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/mod.mk

M:=\
    $(DIR_SRC)/modules/arch/x86/usb/ohci.ko.0 \

O:=\
    $(DIR_SRC)/modules/arch/x86/usb/ohci.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/mod.mk

M:=\
    $(DIR_SRC)/modules/arch/x86/usb/uhci.ko.0 \

O:=\
    $(DIR_SRC)/modules/arch/x86/usb/uhci.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
