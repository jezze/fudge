M:=\
    $(DIR_SRC)/modules/arch/x86/ahci/ahci.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/ahci/ahci.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/ahci/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
