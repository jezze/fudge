M:=\
    $(DIR_SRC)/modules/platform/virt/platform.ko \

N:=\
    $(DIR_SRC)/modules/platform/virt/platform.ko.map \

O:=\
    $(DIR_SRC)/modules/platform/virt/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/platform/virt/virt-timer.ko \

N:=\
    $(DIR_SRC)/modules/platform/virt/virt-timer.ko.map \

O:=\
    $(DIR_SRC)/modules/platform/virt/timer.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
