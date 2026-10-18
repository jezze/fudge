M:=\
    $(DIR_SRC)/modules/icp/icp.ko \

N:=\
    $(DIR_SRC)/modules/icp/icp.ko.map \

O:=\
    $(DIR_SRC)/modules/icp/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/icp/icp-timer.ko \

N:=\
    $(DIR_SRC)/modules/icp/icp-timer.ko.map \

O:=\
    $(DIR_SRC)/modules/icp/timer.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
