M:=\
    $(DIR_SRC)/modules/ahci/ahci.ko \

N:=\
    $(DIR_SRC)/modules/ahci/ahci.ko.map \

O:=\
    $(DIR_SRC)/modules/ahci/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
