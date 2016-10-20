M:=\
    $(DIR_SRC)/modules/arch/x86/ahci/ahci.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/ahci/main.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
