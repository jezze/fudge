M:=\
    $(DIR_SRC)/modules/platform/icp/platform.ko \

N:=\
    $(DIR_SRC)/modules/platform/icp/platform.ko.map \

O:=\
    $(DIR_SRC)/modules/platform/icp/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/platform/icp/icp-timer.ko \

N:=\
    $(DIR_SRC)/modules/platform/icp/icp-timer.ko.map \

O:=\
    $(DIR_SRC)/modules/platform/icp/timer.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
