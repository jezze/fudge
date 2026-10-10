M:=\
    $(DIR_SRC)/modules/cpuid/x86/cpuid.ko \

N:=\
    $(DIR_SRC)/modules/cpuid/x86/cpuid.ko.map \

O:=\
    $(DIR_SRC)/modules/cpuid/main.o \
    $(DIR_SRC)/modules/cpuid/x86/cpuid.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
