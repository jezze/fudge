M:=\
    $(DIR_SRC)/modules/smc91c111/smc91c111.ko \

N:=\
    $(DIR_SRC)/modules/smc91c111/smc91c111.ko.map \

O:=\
    $(DIR_SRC)/modules/smc91c111/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
