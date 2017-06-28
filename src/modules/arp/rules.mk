M:=\
    $(DIR_SRC)/modules/arp/arp.ko \

O:=\
    $(DIR_SRC)/modules/arp/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
