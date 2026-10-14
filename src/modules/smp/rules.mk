M:=\
    $(DIR_SRC)/modules/smp/smp.ko \

N:=\
    $(DIR_SRC)/modules/smp/smp.ko.map \

O:=\
    $(DIR_SRC)/modules/smp/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_SRC)/modules/smp/$(ARCH)/rules.mk
include $(DIR_MK)/kmod.mk
