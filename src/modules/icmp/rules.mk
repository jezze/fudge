M:=\
    $(DIR_SRC)/modules/icmp/icmp.ko \

O:=\
    $(DIR_SRC)/modules/icmp/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
