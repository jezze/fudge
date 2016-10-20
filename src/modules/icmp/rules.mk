M:=\
    $(DIR_SRC)/modules/icmp/icmp.ko \

O:=\
    $(DIR_SRC)/modules/icmp/main.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
