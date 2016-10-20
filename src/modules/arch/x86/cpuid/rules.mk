M:=\
    $(DIR_SRC)/modules/arch/x86/cpuid/cpuid.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/cpuid/main.o \
    $(DIR_SRC)/modules/arch/x86/cpuid/cpuid.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
