M:=\
    $(DIR_SRC)/modules/hpet/hpet.ko \

N:=\
    $(DIR_SRC)/modules/hpet/hpet.ko.map \

O:=\
    $(DIR_SRC)/modules/hpet/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
