M:=\
    $(DIR_SRC)/modules/apic/x86_64/apic.ko \

N:=\
    $(DIR_SRC)/modules/apic/x86_64/apic.ko.map \

O:=\
    $(DIR_SRC)/modules/apic/x86_64/main.o \
    $(DIR_SRC)/modules/apic/x86_64/apic.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
