M:=\
    $(DIR_SRC)/modules/icmp/icmp.ko \

N:=\
    $(DIR_SRC)/modules/icmp/icmp.ko.map \

O:=\
    $(DIR_SRC)/modules/icmp/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
