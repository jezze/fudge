M:=\
    $(DIR_SRC)/modules/cpuid/x86_64/cpuid.ko \

N:=\
    $(DIR_SRC)/modules/cpuid/x86_64/cpuid.ko.map \

O:=\
    $(DIR_SRC)/modules/cpuid/main.o \
    $(DIR_SRC)/modules/cpuid/x86_64/cpuid.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
