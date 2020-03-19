M:=\
    $(DIR_SRC)/modules/arch/x86/ioapic/ioapic.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/ioapic/ioapic.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/ioapic/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
