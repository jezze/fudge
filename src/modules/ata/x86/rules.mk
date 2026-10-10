M:=\
    $(DIR_SRC)/modules/ata/x86/ata.ko \

N:=\
    $(DIR_SRC)/modules/ata/x86/ata.ko.map \

O:=\
    $(DIR_SRC)/modules/ata/x86/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
