M:=\
    $(DIR_SRC)/modules/arch/x86/pci/pci.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/pci/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
