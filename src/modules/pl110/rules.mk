M:=\
    $(DIR_SRC)/modules/pl110/pl110.ko \

N:=\
    $(DIR_SRC)/modules/pl110/pl110.ko.map \

O:=\
    $(DIR_SRC)/modules/pl110/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
