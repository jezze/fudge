M:=\
    $(DIR_SRC)/modules/arp/arp.ko \

O:=\
    $(DIR_SRC)/modules/arp/main.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
