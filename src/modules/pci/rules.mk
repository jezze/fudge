M:=\
    $(DIR_SRC)/modules/pci/pci.ko \

N:=\
    $(DIR_SRC)/modules/pci/pci.ko.map \

O:=\
    $(DIR_SRC)/modules/pci/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
