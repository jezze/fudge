M:=\
    $(DIR_SRC)/modules/smp/x86/smp.ko \

N:=\
    $(DIR_SRC)/modules/smp/x86/smp.ko.map \

O:=\
    $(DIR_SRC)/modules/smp/x86/main.o \
    $(DIR_SRC)/modules/smp/x86/init.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
