M:=\
    $(DIR_SRC)/modules/arch/x86/pci/pci.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/pci/main.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
