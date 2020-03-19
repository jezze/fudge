M:=\
    $(DIR_SRC)/modules/arch/x86/apic/apic.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/apic/apic.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/apic/main.o \
    $(DIR_SRC)/modules/arch/x86/apic/apic.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
