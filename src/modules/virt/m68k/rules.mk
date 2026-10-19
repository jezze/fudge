M:=\
    $(DIR_SRC)/modules/virt/virt.ko \

N:=\
    $(DIR_SRC)/modules/virt/virt.ko.map \

O:=\
    $(DIR_SRC)/modules/virt/m68k/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/virt/virt-timer.ko \

N:=\
    $(DIR_SRC)/modules/virt/virt-timer.ko.map \

O:=\
    $(DIR_SRC)/modules/virt/m68k/timer.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
