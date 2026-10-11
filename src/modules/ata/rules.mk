M:=\
    $(DIR_SRC)/modules/ata/ata.ko \

N:=\
    $(DIR_SRC)/modules/ata/ata.ko.map \

O:=\
    $(DIR_SRC)/modules/ata/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
