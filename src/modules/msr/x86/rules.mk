M:=\
    $(DIR_SRC)/modules/msr/x86/msr.ko \

N:=\
    $(DIR_SRC)/modules/msr/x86/msr.ko.map \

O:=\
    $(DIR_SRC)/modules/msr/main.o \
    $(DIR_SRC)/modules/msr/x86/msr.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
